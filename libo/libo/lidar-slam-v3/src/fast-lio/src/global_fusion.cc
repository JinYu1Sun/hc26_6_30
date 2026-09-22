#include "fusion/global_fusion.h"

#include <algorithm>
#include <iterator>
#include <limits>

namespace
{
inline double WrapPi(double a) { return std::atan2(std::sin(a), std::cos(a)); }
inline double YawOfQuat(const Eigen::Quaterniond &q)
{
    return std::atan2(2.0 * (q.w() * q.z() + q.x() * q.y()),
                      1.0 - 2.0 * (q.y() * q.y() + q.z() * q.z()));
}
} // namespace

GlobalOptimization::GlobalOptimization()
{
    clear_data = false;

    hasLastLio_ = false;

    theta_ = 0.0;
    tx_ = 0.0;
    ty_ = 0.0;
    aligned_ = false;
    locked_ = false;

    last_enu_yaw_ = 0.0;
    last_lio_yaw_ = 0.0;
    last_yaw_valid_ = false;

    lastLioP_.setZero();
    lastLioQ_.setIdentity();
    lastGlobalP_.setZero();
    lastGlobalQ_.setIdentity();

    // 参数默认值（以精度为主：更长窗口多平均、拐弯清晰才锁定；冷启动由 RTK 航向种子兜底）
    max_window_ = 400;       // 约覆盖 window_span × GPS频率
    max_window_span_ = 45.0; // s，更长 → 更平滑、长直线段也能保住上一次拐弯的锁定
    match_time_tol_ = 0.05;  // s，与 fusion 中 ±0.08 同步窗一致量级
    seed_along_std_ = 0.50;  // m，沿迹基线达此值才用窗口平均方向（更稳）
    lock_cross_std_ = 0.35;  // m，需较清晰拐弯才锁定旋转（避免欠观测误锁）
    lio_cache_max_ = 1200;
}

GlobalOptimization::~GlobalOptimization() {}

void GlobalOptimization::ApplyClearIfRequested_()
{
    if (!clear_data)
        return;
    window_.clear();
    lioPoseMap_.clear();
    hasLastLio_ = false;
    aligned_ = false;
    locked_ = false;
    theta_ = 0.0;
    tx_ = 0.0;
    ty_ = 0.0;
    clear_data = false;
    std::cout << "[global_fusion] clear_data handled (full reset)." << std::endl;
}

void GlobalOptimization::ResetLioCoupled()
{
    std::lock_guard<std::mutex> lk(mtx_);
    // 仅清 LIO 耦合状态（配对窗口/对齐/最近 LIO）。估计器已无 GPS 原点需保留。
    window_.clear();
    lioPoseMap_.clear();
    hasLastLio_ = false;
    aligned_ = false;
    locked_ = false;
    theta_ = 0.0;
    tx_ = 0.0;
    ty_ = 0.0;
    lastGlobalP_.setZero();
    lastGlobalQ_.setIdentity();
    std::cout << "[global_fusion] ResetLioCoupled (keep GPS origin)." << std::endl;
}

void GlobalOptimization::InputOdom(double t, Eigen::Vector3d OdomP, Eigen::Quaterniond OdomQ)
{
    std::lock_guard<std::mutex> lk(mtx_);
    ApplyClearIfRequested_();

    lastLioP_ = OdomP;
    lastLioQ_ = OdomQ.normalized();
    last_lio_yaw_ = YawOfQuat(lastLioQ_); // 供 RTK 航向冷启动种子配对
    hasLastLio_ = true;

    lioPoseMap_[t] = {OdomP.x(), OdomP.y(), OdomP.z()};
    while (lioPoseMap_.size() > lio_cache_max_)
        lioPoseMap_.erase(lioPoseMap_.begin());

    // 以当前（可能为冻结的）SE(2) 输出 LIO 回退位姿。
    RemapLastGlobal_();
}

void GlobalOptimization::InputGPS(double t, double enu_x, double enu_y, double enu_yaw, bool yaw_valid)
{
    std::lock_guard<std::mutex> lk(mtx_);
    ApplyClearIfRequested_();

    last_enu_yaw_ = enu_yaw;     // 供冷启动航向种子
    last_yaw_valid_ = yaw_valid; // 上层按速度门控的 RTK 航向可信标志

    // enu_x/enu_y 已是上层 GPS2Local 输出的 ENU 平面位置（同一帧），直接配对。
    if (lioPoseMap_.empty())
        return;

    // 找到与 t 时间最近的 LIO 位姿（lower_bound 及其前一个里取更近者）。
    auto it = lioPoseMap_.lower_bound(t);
    double best_dt = std::numeric_limits<double>::max();
    const std::array<double, 3> *best = nullptr;
    if (it != lioPoseMap_.end())
    {
        double dt = std::fabs(it->first - t);
        if (dt < best_dt)
        {
            best_dt = dt;
            best = &it->second;
        }
    }
    if (it != lioPoseMap_.begin())
    {
        auto p = std::prev(it);
        double dt = std::fabs(p->first - t);
        if (dt < best_dt)
        {
            best_dt = dt;
            best = &p->second;
        }
    }
    if (best == nullptr || best_dt > match_time_tol_)
        return; // 没有足够同步的 LIO 帧，本帧不入窗

    MatchPair mp;
    mp.t = t;
    mp.lx = (*best)[0];
    mp.ly = (*best)[1];
    mp.mx = enu_x;
    mp.my = enu_y;
    window_.push_back(mp);

    // 修剪窗口：超长度或超时间跨度。
    while (window_.size() > max_window_)
        window_.pop_front();
    while (window_.size() >= 2 && (window_.back().t - window_.front().t) > max_window_span_)
        window_.pop_front();

    UpdateAlignment_();
    RemapLastGlobal_(); // GPS 更新了 (theta,t) 后，刷新输出，避免一帧滞后
}

// 闭式 2D 刚体（SE(2)）对齐：求 R(theta_),t 使 p_map ≈ R(theta_)·p_lio + t。
void GlobalOptimization::UpdateAlignment_()
{
    const size_t n = window_.size();
    if (n < 1)
        return; // n==1 时展布为 0，将走 RTK 航向冷启动种子分支

    double cx_l = 0, cy_l = 0, cx_m = 0, cy_m = 0;
    for (const auto &p : window_)
    {
        cx_l += p.lx;
        cy_l += p.ly;
        cx_m += p.mx;
        cy_m += p.my;
    }
    cx_l /= n;
    cy_l /= n;
    cx_m /= n;
    cy_m /= n;

    double Sxx = 0, Sxy = 0, Syx = 0, Syy = 0; // 旋转互相关
    double Saxx = 0, Saxy = 0, Sayy = 0;       // 居中 LIO 点的自相关（观测性）
    for (const auto &p : window_)
    {
        double ax = p.lx - cx_l, ay = p.ly - cy_l;
        double bx = p.mx - cx_m, by = p.my - cy_m;
        Sxx += ax * bx;
        Sxy += ax * by;
        Syx += ay * bx;
        Syy += ay * by;
        Saxx += ax * ax;
        Saxy += ax * ay;
        Sayy += ay * ay;
    }

    // LIO 点云 2D 展布：协方差特征值开方 = 沿/横迹向标准差。
    double Cxx = Saxx / n, Cxy = Saxy / n, Cyy = Sayy / n;
    double tr = Cxx + Cyy;
    double det = Cxx * Cyy - Cxy * Cxy;
    double disc = std::sqrt(std::max(0.0, tr * tr / 4.0 - det));
    double lmax = tr / 2.0 + disc;
    double lmin = tr / 2.0 - disc;
    double along_std = std::sqrt(std::max(0.0, lmax));
    double cross_std = std::sqrt(std::max(0.0, lmin));

    double theta_new = std::atan2(Sxy - Syx, Sxx + Syy);

    // 旋转来源按精度优先级选取（高→低）：
    //   1) 横迹展布充分(经历拐弯)  → 全 Umeyama，锁定（最精确，用全部点的二维拟合）
    //   2) 已锁定后的近直线段       → 保持冻结 theta_（不被直线拉偏）
    //   3) 沿迹基线达标            → 窗口平均的运动方向（比单航向稳）
    //   4) RTK 航向可信            → 冷启动种子 theta = ENU航向 - LIO航向（短时短距足够）
    //   否则                       → 暂不可对齐（既无锁定、又无基线、又无有效航向）
    bool have_theta = false;
    if (cross_std >= lock_cross_std_)
    {
        theta_ = theta_new;
        locked_ = true;
        have_theta = true;
    }
    else if (locked_)
    {
        have_theta = true; // 保持已锁定的 theta_
    }
    else if (along_std >= seed_along_std_)
    {
        theta_ = theta_new; // 窗口平均运动方向（近直线但基线足够）
        have_theta = true;
    }
    else if (last_yaw_valid_)
    {
        theta_ = WrapPi(last_enu_yaw_ - last_lio_yaw_); // RTK 航向冷启动种子
        have_theta = true;
    }

    if (!have_theta)
        return; // 既未锁、又无基线、又无有效 RTK 航向：本帧维持原状、暂不对齐

    // 平移：用当前 theta_ 对齐质心（n==1 时质心即该单点，等价锚到当前 GPS）。
    double c = cos(theta_), s = sin(theta_);
    tx_ = cx_m - (c * cx_l - s * cy_l);
    ty_ = cy_m - (s * cx_l + c * cy_l);

    aligned_ = true;
}

void GlobalOptimization::RemapLastGlobal_()
{
    if (!hasLastLio_)
        return;
    double c = cos(theta_), s = sin(theta_);
    lastGlobalP_.x() = c * lastLioP_.x() - s * lastLioP_.y() + tx_;
    lastGlobalP_.y() = s * lastLioP_.x() + c * lastLioP_.y() + ty_;
    lastGlobalP_.z() = lastLioP_.z();
    Eigen::Quaterniond rz(Eigen::AngleAxisd(theta_, Eigen::Vector3d::UnitZ()));
    lastGlobalQ_ = (rz * lastLioQ_).normalized();
}

void GlobalOptimization::GetGlobalOdom(Eigen::Vector3d &odomP, Eigen::Quaterniond &odomQ)
{
    std::lock_guard<std::mutex> lk(mtx_);
    odomP = lastGlobalP_;
    odomQ = lastGlobalQ_;
}

bool GlobalOptimization::IsAligned() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return aligned_;
}

bool GlobalOptimization::IsLocked() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return locked_;
}

bool GlobalOptimization::GetTransform(double &theta, double &tx, double &ty) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    theta = theta_;
    tx = tx_;
    ty = ty_;
    return aligned_;
}

void GlobalOptimization::SetParams(int max_window, double window_span_sec, double match_tol_sec,
                                   double seed_along_std, double lock_cross_std)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (max_window >= 2)
        max_window_ = static_cast<size_t>(max_window);
    if (window_span_sec > 0.0)
        max_window_span_ = window_span_sec;
    if (match_tol_sec > 0.0)
        match_time_tol_ = match_tol_sec;
    if (seed_along_std > 0.0)
        seed_along_std_ = seed_along_std;
    if (lock_cross_std > 0.0)
        lock_cross_std_ = lock_cross_std;
}
