#include "fusion/global_fusion.h"
#include "fusion/lio_time_alignment.h"

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
    clear_data_ = false;

    hasLastLio_ = false;
    hasLastMatchedLio_ = false;

    theta_ = 0.0;
    tx_ = 0.0;
    ty_ = 0.0;
    tz_ = 0.0;
    aligned_ = false;
    locked_ = false;
    vertical_aligned_ = false;
    lock_confirm_cnt_ = 0;
    last_lock_confirmation_t_ = -std::numeric_limits<double>::infinity();
    last_lock_confirmation_lx_ = 0.0;
    last_lock_confirmation_ly_ = 0.0;
    pending_lock_transform_valid_ = false;
    pending_lock_theta_ = 0.0;
    pending_lock_tx_ = 0.0;
    pending_lock_ty_ = 0.0;

    last_fit_residual_rms_ = 0.0;
    last_outlier_ratio_ = 0.0;
    last_vertical_std_ = 0.0;
    last_vertical_outlier_ratio_ = 0.0;
    last_vertical_inlier_count_ = 0;
    last_along_std_ = 0.0;
    last_cross_std_ = 0.0;
    last_window_n_ = 0;
    last_horizontal_inlier_count_ = 0;
    last_lock_condition_number_ = std::numeric_limits<double>::infinity();
    last_accumulated_turn_rad_ = 0.0;

    last_enu_yaw_ = 0.0;
    last_lio_yaw_ = 0.0;
    last_yaw_valid_ = false;

    last_enu_x_ = 0.0;
    last_enu_y_ = 0.0;
    last_enu_z_ = 0.0;
    last_z_valid_ = false;
    has_last_gps_ = false;

    lastLioP_.setZero();
    lastLioQ_.setIdentity();
    lastMatchedLioP_.setZero();
    lastGlobalP_.setZero();
    lastGlobalQ_.setIdentity();

    // 参数默认值（以精度为主：更长窗口多平均、拐弯清晰才锁定；冷启动由 RTK 航向种子兜底）
    max_window_ = 400;       // 约覆盖 window_span × GPS频率
    max_window_span_ = 45.0; // s，更长 → 更平滑、长直线段也能保住上一次拐弯的锁定
    match_time_tol_ = 0.08;  // s，与 fusion 上层队列共用 se2_match_tol_sec
    max_lio_pair_dt_sec_ = 0.60;
    max_lio_extrapolation_sec_ = 0.05;
    seed_along_std_ = 0.50;  // m，沿迹基线达此值才用窗口平均方向（更稳）
    lock_cross_std_ = 0.10;  // m，按本机0.4m行距与实测直线噪声设定
    lio_cache_max_ = 1200;

    // 锁定稳健性 + 鲁棒拟合默认值：
    lock_min_samples_ = 15;         // 至少 15 个鲁棒内点才允许提交锁定（防小样本误锁）
    lock_confirm_frames_ = 3;       // cross_std 需3组独立观测达标才提交；
                                     // 真实拐弯的 cross_std 抬升会持续覆盖窗口跨度内的多帧，
                                     // 正常拐弯轻松满足，不影响可用性。
    outlier_residual_thresh_ = 0.5; // m，鲁棒重拟合的残差剔除阈值（RTK/LIO 正常噪声远小于此）
    innovation_unlocked_xy_m_ = 0.75;
    innovation_locked_xy_m_ = 0.40;
    innovation_yaw_rad_ = 15.0 * M_PI / 180.0;
    innovation_z_m_ = 0.75;
    lock_confirmation_min_interval_sec_ = 1.0;
    lock_confirmation_min_displacement_m_ = 0.30;
    lock_min_inlier_ratio_ = 0.70;
    lock_min_along_std_m_ = 0.50;
    lock_max_condition_number_ = 50.0;
    lock_min_accumulated_turn_rad_ = 30.0 * M_PI / 180.0;
    vertical_min_samples_ = 10;
    vertical_max_std_ = 0.30;          // m, RTK/INS高程需现场标定
    vertical_outlier_threshold_ = 0.80; // m
    vertical_max_abs_offset_ = 1000.0;  // m, 拒绝单位/基准明显错误
}

GlobalOptimization::~GlobalOptimization() {}

void GlobalOptimization::ApplyClearIfRequested_()
{
    if (!clear_data_)
        return;
    window_.clear();
    lioPoseMap_.clear();
    hasLastLio_ = false;
    hasLastMatchedLio_ = false;
    aligned_ = false;
    locked_ = false;
    lock_confirm_cnt_ = 0;
    last_lock_confirmation_t_ = -std::numeric_limits<double>::infinity();
    pending_lock_transform_valid_ = false;
    last_fit_residual_rms_ = 0.0;
    last_outlier_ratio_ = 0.0;
    last_along_std_ = 0.0;
    last_cross_std_ = 0.0;
    last_window_n_ = 0;
    last_horizontal_inlier_count_ = 0;
    last_lock_condition_number_ = std::numeric_limits<double>::infinity();
    last_accumulated_turn_rad_ = 0.0;
    theta_ = 0.0;
    tx_ = 0.0;
    ty_ = 0.0;
    tz_ = 0.0;
    vertical_aligned_ = false;
    last_vertical_std_ = 0.0;
    last_vertical_outlier_ratio_ = 0.0;
    last_vertical_inlier_count_ = 0;
    ClearLastGpsSeedUnlocked_();
    clear_data_ = false;
    std::cout << "[global_fusion] clear_data handled (full reset)." << std::endl;
}

void GlobalOptimization::RequestClear()
{
    std::lock_guard<std::mutex> lk(mtx_);
    clear_data_ = true;
}

void GlobalOptimization::ResetLioCoupledUnlocked_()
{
    window_.clear();
    lioPoseMap_.clear();
    hasLastLio_ = false;
    hasLastMatchedLio_ = false;
    aligned_ = false;
    locked_ = false;
    lock_confirm_cnt_ = 0;
    last_lock_confirmation_t_ = -std::numeric_limits<double>::infinity();
    pending_lock_transform_valid_ = false;
    last_fit_residual_rms_ = 0.0;
    last_outlier_ratio_ = 0.0;
    last_along_std_ = 0.0;
    last_cross_std_ = 0.0;
    last_window_n_ = 0;
    last_horizontal_inlier_count_ = 0;
    last_lock_condition_number_ = std::numeric_limits<double>::infinity();
    last_accumulated_turn_rad_ = 0.0;
    theta_ = 0.0;
    tx_ = 0.0;
    ty_ = 0.0;
    tz_ = 0.0;
    vertical_aligned_ = false;
    last_vertical_std_ = 0.0;
    last_vertical_outlier_ratio_ = 0.0;
    last_vertical_inlier_count_ = 0;
    lastGlobalP_.setZero();
    lastGlobalQ_.setIdentity();
    lastMatchedLioP_.setZero();
}

void GlobalOptimization::ClearLastGpsSeedUnlocked_()
{
    last_enu_yaw_ = 0.0;
    last_lio_yaw_ = 0.0;
    last_yaw_valid_ = false;
    last_enu_x_ = 0.0;
    last_enu_y_ = 0.0;
    last_enu_z_ = 0.0;
    last_z_valid_ = false;
    has_last_gps_ = false;
    hasLastMatchedLio_ = false;
}

void GlobalOptimization::ResetLioCoupled()
{
    std::lock_guard<std::mutex> lk(mtx_);
    // LIO 代际复位时保留最近 GPS，新代 LIO 可借它快速播种；
    // 地图切换不得使用此接口，必须调 ResetForMapSwitch。
    ResetLioCoupledUnlocked_();
    std::cout << "[global_fusion] ResetLioCoupled (keep GPS origin)." << std::endl;
}

void GlobalOptimization::ResetForMapSwitch()
{
    std::lock_guard<std::mutex> lk(mtx_);
    ResetLioCoupledUnlocked_();
    ClearLastGpsSeedUnlocked_();
    std::cout << "[global_fusion] ResetForMapSwitch (clear old-map GPS seed)."
              << std::endl;
}

void GlobalOptimization::ClearGpsWindowKeepTransform()
{
    std::lock_guard<std::mutex> lk(mtx_);
    window_.clear();
    lock_confirm_cnt_ = 0;
    last_lock_confirmation_t_ = -std::numeric_limits<double>::infinity();
    pending_lock_transform_valid_ = false;
    last_fit_residual_rms_ = 0.0;
    last_outlier_ratio_ = 0.0;
    last_along_std_ = 0.0;
    last_cross_std_ = 0.0;
    last_window_n_ = 0;
    last_horizontal_inlier_count_ = 0;
    last_lock_condition_number_ = std::numeric_limits<double>::infinity();
    last_accumulated_turn_rad_ = 0.0;
    last_vertical_std_ = 0.0;
    last_vertical_outlier_ratio_ = 0.0;
    last_vertical_inlier_count_ = 0;
    last_yaw_valid_ = false;
    has_last_gps_ = false;
    hasLastMatchedLio_ = false;
    lastMatchedLioP_.setZero();
    last_z_valid_ = false;
}

void GlobalOptimization::InputOdom(double t, Eigen::Vector3d OdomP, Eigen::Quaterniond OdomQ)
{
    std::lock_guard<std::mutex> lk(mtx_);
    ApplyClearIfRequested_();

    if (!std::isfinite(t) || !OdomP.allFinite() ||
        !OdomQ.coeffs().allFinite() || OdomQ.squaredNorm() < 1e-12)
        return;

    lastLioP_ = OdomP;
    lastLioQ_ = OdomQ.normalized();
    const double current_lio_yaw = YawOfQuat(lastLioQ_);
    hasLastLio_ = true;

    lioPoseMap_[t] = {OdomP.x(), OdomP.y(), OdomP.z(), current_lio_yaw};
    while (lioPoseMap_.size() > lio_cache_max_)
        lioPoseMap_.erase(lioPoseMap_.begin());

    // 以当前（可能为冻结的）SE(2) 输出 LIO 回退位姿。
    RemapLastGlobal_();
}

bool GlobalOptimization::InputGPS(double t, double enu_x, double enu_y,
                                  double enu_z, double enu_yaw,
                                  bool yaw_valid, bool z_valid,
                                  bool commit_alignment)
{
    std::lock_guard<std::mutex> lk(mtx_);
    ApplyClearIfRequested_();

    if (!std::isfinite(t) || !std::isfinite(enu_x) ||
        !std::isfinite(enu_y) || (yaw_valid && !std::isfinite(enu_yaw)))
        return false;

    // enu_x/enu_y 已是上层 GPS2Local 输出的 ENU 平面位置（同一帧），直接配对。
    if (lioPoseMap_.empty())
        return false;

    // Use a LIO pose at the GPS *measurement time*, not the nearest LIO
    // sample. The nearest-sample error otherwise enters both the innovation
    // gate and the 4DoF fitting window even if publication uses aligned time.
    const auto after = lioPoseMap_.lower_bound(t);
    std::array<double, 4> matched;
    if (after != lioPoseMap_.end() && after->first == t)
    {
        matched = after->second;
    }
    else
    {
        auto first = after;
        auto second = after;
        if (after == lioPoseMap_.begin())
        {
            // A GPS sample before the oldest cached LIO needs two samples
            // for bounded backward extrapolation; never freeze one frame.
            if (lioPoseMap_.size() < 2) return false;
            second = std::next(first);
        }
        else if (after == lioPoseMap_.end())
        {
            // GPS is just ahead of the newest LIO: bounded extrapolation.
            if (lioPoseMap_.size() < 2) return false;
            second = std::prev(lioPoseMap_.end());
            first = std::prev(second);
        }
        else
        {
            // Bracketed GPS timestamp: interpolate the H-frame base pose.
            second = after;
            first = std::prev(after);
        }
        if (std::min(std::fabs(first->first - t),
                     std::fabs(second->first - t)) > match_time_tol_ + 1e-9)
            return false;
        const auto make_timed = [](decltype(first) sample) {
            fusion_4dof::TimedLioRawPose pose;
            pose.valid = true; // cache is cleared on every LIO generation reset
            pose.stamp = sample->first;
            pose.x = sample->second[0];
            pose.y = sample->second[1];
            pose.z = sample->second[2];
            pose.yaw = sample->second[3];
            return pose;
        };
        fusion_4dof::TimedLioRawPose aligned;
        if (!fusion_4dof::LioPoseAtMeasurementTime(
                make_timed(first), make_timed(second), t, &aligned,
                max_lio_pair_dt_sec_,
                std::min(match_time_tol_, max_lio_extrapolation_sec_)))
            return false;
        matched = {aligned.x, aligned.y, aligned.z, aligned.yaw};
    }
    const std::array<double, 4> *best = &matched;

    // Gate against the immutable, previously committed transform before this
    // sample is allowed to influence either the seed or the fitting window.
    if (aligned_)
    {
        const double c = std::cos(theta_), s = std::sin(theta_);
        const double predicted_x = c * (*best)[0] - s * (*best)[1] + tx_;
        const double predicted_y = s * (*best)[0] + c * (*best)[1] + ty_;
        const double xy_innovation =
            std::hypot(enu_x - predicted_x, enu_y - predicted_y);
        const double xy_limit =
            locked_ ? innovation_locked_xy_m_ : innovation_unlocked_xy_m_;
        const double yaw_innovation =
            yaw_valid ? std::fabs(WrapPi(enu_yaw - WrapPi((*best)[3] + theta_)))
                      : 0.0;
        const double z_innovation =
            (vertical_aligned_ && z_valid && std::isfinite(enu_z))
                ? std::fabs(enu_z - ((*best)[2] + tz_))
                : 0.0;
        if (!std::isfinite(xy_innovation) || xy_innovation > xy_limit ||
            (yaw_valid && yaw_innovation > innovation_yaw_rad_) ||
            (vertical_aligned_ && z_valid && z_innovation > innovation_z_m_))
        {
            return false;
        }
    }

    last_enu_yaw_ = yaw_valid ? enu_yaw : 0.0;
    last_lio_yaw_ = (*best)[3];
    lastMatchedLioP_ = Eigen::Vector3d((*best)[0], (*best)[1], (*best)[2]);
    hasLastMatchedLio_ = true;
    last_yaw_valid_ = yaw_valid;
    last_enu_x_ = enu_x;
    last_enu_y_ = enu_y;
    last_enu_z_ = enu_z;
    last_z_valid_ = z_valid && std::isfinite(enu_z);
    has_last_gps_ = true;

    MatchPair mp;
    mp.t = t;
    mp.lx = (*best)[0];
    mp.ly = (*best)[1];
    mp.lz = (*best)[2];
    mp.lyaw = (*best)[3];
    mp.mx = enu_x;
    mp.my = enu_y;
    mp.mz = enu_z;
    mp.z_valid = last_z_valid_ && std::isfinite(mp.lz);
    window_.push_back(mp);

    // 修剪窗口：超长度或超时间跨度。
    while (window_.size() > max_window_)
        window_.pop_front();
    while (window_.size() >= 2 && (window_.back().t - window_.front().t) > max_window_span_)
        window_.pop_front();

    if (commit_alignment)
    {
        UpdateVerticalAlignment_();
        UpdateAlignment_();
        RemapLastGlobal_(); // GPS 更新了 (theta,t) 后，刷新输出，避免一帧滞后
    }
    return true;
}

// 对一组配对点做一次闭式 2D Umeyama 拟合：质心、沿/横迹标准差、旋转角。
// 私有静态方法，使 UpdateAlignment_ 能对“全窗口”与“剔除异常值后的内点”
// 各调用一次，逻辑不重复（MatchPair 是私有嵌套类型，故不能用自由函数）。
void GlobalOptimization::FitUmeyama2D_(const std::deque<MatchPair> &pts, double &cx_l,
                                       double &cy_l, double &cx_m, double &cy_m,
                                       double &along_std, double &cross_std,
                                       double &theta_out)
{
    const size_t m = pts.size();
    cx_l = cy_l = cx_m = cy_m = 0.0;
    for (const auto &p : pts)
    {
        cx_l += p.lx;
        cy_l += p.ly;
        cx_m += p.mx;
        cy_m += p.my;
    }
    cx_l /= m;
    cy_l /= m;
    cx_m /= m;
    cy_m /= m;

    double Sxx = 0, Sxy = 0, Syx = 0, Syy = 0; // 旋转互相关
    double Saxx = 0, Saxy = 0, Sayy = 0;       // 居中 LIO 点的自相关（观测性）
    for (const auto &p : pts)
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
    double Cxx = Saxx / m, Cxy = Saxy / m, Cyy = Sayy / m;
    double tr = Cxx + Cyy;
    double det = Cxx * Cyy - Cxy * Cxy;
    double disc = std::sqrt(std::max(0.0, tr * tr / 4.0 - det));
    along_std = std::sqrt(std::max(0.0, tr / 2.0 + disc));
    cross_std = std::sqrt(std::max(0.0, tr / 2.0 - disc));
    theta_out = std::atan2(Sxy - Syx, Sxx + Syy);
}

// 闭式 2D 刚体（SE(2)）对齐：求 R(theta_),t 使 p_map ≈ R(theta_)·p_lio + t。
// 含一次异常值鲁棒重拟合与“锁定”确认门槛：
//   1) 全窗口做参考拟合；
//   2) 用参考变换剔除残差超阈值的点，内点占多数时用内点重新拟合（覆盖参考值），
//      内点残差 RMS 同时作为拟合质量自检输出（GetFitResidualRms/GetOutlierRatio）；
//   3) “锁定”须同时满足：横迹展布达标 + 窗口样本量达标 + 本帧数据可信（未触发
//      多数异常值），且 lock_confirm_frames_ 组时间/位移独立的观测都满足才提交，
//      防止相邻重叠滑窗被误当作独立证据。
void GlobalOptimization::UpdateAlignment_()
{
    // `locked_` is a production freeze, not merely a historical quality bit.
    // A separate shadow estimator may be added later, but RTK can never
    // rewrite the transform used by state 2 after lock.
    if (locked_)
        return;
    const size_t n = window_.size();
    if (n < 1)
        return; // n==1 时展布为 0，将走 RTK 航向冷启动种子分支

    double cx_l, cy_l, cx_m, cy_m, along_std, cross_std, theta_new;
    FitUmeyama2D_(window_, cx_l, cy_l, cx_m, cy_m, along_std, cross_std, theta_new);

    // ---- 异常值鲁棒重拟合 + 拟合质量自检 ----
    std::deque<MatchPair> fit_inliers = window_;
    if (n >= 4)
    {
        double c0 = std::cos(theta_new), s0 = std::sin(theta_new);
        double tx0 = cx_m - (c0 * cx_l - s0 * cy_l);
        double ty0 = cy_m - (s0 * cx_l + c0 * cy_l);

        std::deque<MatchPair> inliers;
        for (const auto &p : window_)
        {
            double px = c0 * p.lx - s0 * p.ly + tx0;
            double py = s0 * p.lx + c0 * p.ly + ty0;
            if (std::hypot(p.mx - px, p.my - py) <= outlier_residual_thresh_)
                inliers.push_back(p);
        }

        last_outlier_ratio_ = static_cast<double>(n - inliers.size()) / static_cast<double>(n);

        if (inliers.size() >= 3 && inliers.size() * 2 > n)
        {
            // 内点占多数：用内点重新拟合，作为本帧的工作估计，覆盖参考值。
            double cx_l2, cy_l2, cx_m2, cy_m2, along2, cross2, theta2;
            FitUmeyama2D_(inliers, cx_l2, cy_l2, cx_m2, cy_m2, along2, cross2, theta2);
            fit_inliers = inliers;
            cx_l = cx_l2;
            cy_l = cy_l2;
            cx_m = cx_m2;
            cy_m = cy_m2;
            along_std = along2;
            cross_std = cross2;
            theta_new = theta2;

            // 内点残差 RMS：用鲁棒重拟合结果重新评估内点，作为质量自检输出。
            double c1 = std::cos(theta_new), s1 = std::sin(theta_new);
            double tx1 = cx_m - (c1 * cx_l - s1 * cy_l);
            double ty1 = cy_m - (s1 * cx_l + c1 * cy_l);
            double sq_sum = 0.0;
            for (const auto &p : inliers)
            {
                double px = c1 * p.lx - s1 * p.ly + tx1;
                double py = s1 * p.lx + c1 * p.ly + ty1;
                double dx = p.mx - px, dy = p.my - py;
                sq_sum += dx * dx + dy * dy;
            }
            last_fit_residual_rms_ = std::sqrt(sq_sum / static_cast<double>(inliers.size()));
        }
        else
        {
            // 内点不足多数：本帧数据系统性异常（而非孤立野点，如真实急转弯瞬时
            // 噪声本身较大），拒绝本次变换提交，保持生产变换；且不计入锁定确认。
            // 这是生产变换的提交门，而不只是“锁定确认”门。旧逻辑虽然把本帧
            // 标成不可信，后面仍会用包含异常点的参考拟合更新 tx/ty，曾在
            // outlier_ratio=1.00 时直接拉动状态 2 输出。多数异常时必须保持
            // 上一次已提交变换不动。
            last_along_std_ = along_std;
            last_cross_std_ = cross_std;
            last_window_n_ = n;
            last_fit_residual_rms_ = std::numeric_limits<double>::infinity();
            lock_confirm_cnt_ = 0;
            last_lock_confirmation_t_ =
                -std::numeric_limits<double>::infinity();
            pending_lock_transform_valid_ = false;
            std::cout << "[global_fusion] majority of window flagged as outliers ("
                      << (n - inliers.size()) << "/" << n
                      << "); reject alignment update and keep committed transform."
                      << std::endl;
            return;
        }
    }
    else
    {
        last_outlier_ratio_ = 0.0; // 样本太少（<4），不评估异常值
    }

    // 调参诊断量：无论本帧是否最终成功对齐，都记录本次几何展布，供离线核对
    // se2_seed_along_std/se2_lock_cross_std/se2_lock_min_samples 是否合适。
    last_along_std_ = along_std;
    last_cross_std_ = cross_std;
    last_window_n_ = n;

    // 旋转来源按精度优先级选取（高→低）：
    //   1) 横迹展布充分(经历拐弯) 且 样本量达标 且 本帧可信 且独立确认数达标
    //      → 全 Umeyama（鲁棒重拟合后），锁定（最精确）
    //   2) 已锁定后的近直线段       → 保持冻结 theta_（不被直线拉偏）
    //   3) 沿迹基线达标            → 窗口平均的运动方向（比单航向稳）
    //   4) RTK 航向可信            → 冷启动种子 theta = ENU航向 - LIO航向（短时短距足够）
    //   否则                       → 暂不可对齐（既无锁定、又无基线、又无有效航向）
    const double candidate_c = std::cos(theta_new);
    const double candidate_s = std::sin(theta_new);
    const double candidate_tx =
        cx_m - (candidate_c * cx_l - candidate_s * cy_l);
    const double candidate_ty =
        cy_m - (candidate_s * cx_l + candidate_c * cy_l);
    // Use the range of unwrapped heading, not sum(abs(delta yaw)). The latter
    // grows without bound under stationary yaw jitter and can eventually fake
    // a real turn. Heading excursion requires the trajectory to have actually
    // occupied headings separated by the configured angle; it also recognizes
    // an S-turn that later returns to its initial heading.
    double accumulated_turn_rad = 0.0;
    if (!fit_inliers.empty())
    {
        double unwrapped_yaw = fit_inliers.front().lyaw;
        double min_unwrapped_yaw = unwrapped_yaw;
        double max_unwrapped_yaw = unwrapped_yaw;
        for (size_t i = 1; i < fit_inliers.size(); ++i)
        {
            unwrapped_yaw +=
                WrapPi(fit_inliers[i].lyaw - fit_inliers[i - 1].lyaw);
            min_unwrapped_yaw = std::min(min_unwrapped_yaw, unwrapped_yaw);
            max_unwrapped_yaw = std::max(max_unwrapped_yaw, unwrapped_yaw);
        }
        accumulated_turn_rad = max_unwrapped_yaw - min_unwrapped_yaw;
    }
    const double inlier_ratio =
        n > 0 ? static_cast<double>(fit_inliers.size()) /
                    static_cast<double>(n)
              : 0.0;
    // Covariance eigenvalues are variances, while along_std/cross_std are
    // their square roots. The requested degeneracy condition number is
    // therefore lambda_max/lambda_min = along_std^2/cross_std^2.
    const double condition_number =
        (along_std * along_std) /
        std::max(cross_std * cross_std, 1.0e-12);
    last_horizontal_inlier_count_ = fit_inliers.size();
    last_lock_condition_number_ = condition_number;
    last_accumulated_turn_rad_ = accumulated_turn_rad;
    bool lock_candidate =
        vertical_aligned_ && fit_inliers.size() >= lock_min_samples_ &&
        inlier_ratio >= lock_min_inlier_ratio_ &&
        along_std >= lock_min_along_std_m_ &&
        cross_std >= lock_cross_std_ &&
        condition_number <= lock_max_condition_number_ &&
        accumulated_turn_rad >= lock_min_accumulated_turn_rad_;
    if (!lock_candidate)
    {
        lock_confirm_cnt_ = 0;
        last_lock_confirmation_t_ = -std::numeric_limits<double>::infinity();
        pending_lock_transform_valid_ = false;
    }
    else
    {
        const MatchPair &newest = window_.back();
        const bool first_confirmation = lock_confirm_cnt_ == 0;
        const bool independently_spaced =
            newest.t - last_lock_confirmation_t_ >=
                lock_confirmation_min_interval_sec_ &&
            std::hypot(newest.lx - last_lock_confirmation_lx_,
                       newest.ly - last_lock_confirmation_ly_) >=
                lock_confirmation_min_displacement_m_;
        if (first_confirmation)
        {
            // Freeze the transform hypothesized by the first geometrically
            // observable window. Later confirmations are new, spaced points
            // tested against this pre-existing hypothesis, not three counts
            // of nearly the same refitted window.
            pending_lock_theta_ = theta_new;
            pending_lock_tx_ = candidate_tx;
            pending_lock_ty_ = candidate_ty;
            pending_lock_transform_valid_ = true;
            lock_confirm_cnt_ = 1;
            last_lock_confirmation_t_ = newest.t;
            last_lock_confirmation_lx_ = newest.lx;
            last_lock_confirmation_ly_ = newest.ly;
        }
        else if (independently_spaced && pending_lock_transform_valid_)
        {
            const double pc = std::cos(pending_lock_theta_);
            const double ps = std::sin(pending_lock_theta_);
            const double predicted_x =
                pc * newest.lx - ps * newest.ly + pending_lock_tx_;
            const double predicted_y =
                ps * newest.lx + pc * newest.ly + pending_lock_ty_;
            const double held_out_residual =
                std::hypot(newest.mx - predicted_x, newest.my - predicted_y);
            const double candidate_translation_change =
                std::hypot(candidate_tx - pending_lock_tx_,
                           candidate_ty - pending_lock_ty_);
            const double candidate_yaw_change =
                std::fabs(WrapPi(theta_new - pending_lock_theta_));
            if (held_out_residual <= outlier_residual_thresh_ &&
                candidate_translation_change <= outlier_residual_thresh_ &&
                candidate_yaw_change <= innovation_yaw_rad_)
            {
                lock_confirm_cnt_ =
                    std::min(lock_confirm_cnt_ + 1, lock_confirm_frames_);
                last_lock_confirmation_t_ = newest.t;
                last_lock_confirmation_lx_ = newest.lx;
                last_lock_confirmation_ly_ = newest.ly;
            }
            else
            {
                lock_confirm_cnt_ = 0;
                last_lock_confirmation_t_ =
                    -std::numeric_limits<double>::infinity();
                pending_lock_transform_valid_ = false;
            }
        }
    }

    bool have_theta = false;
    if (lock_confirm_cnt_ >= lock_confirm_frames_)
    {
        theta_ = theta_new;
        locked_ = true;
        have_theta = true;
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

    // 平移：用当前 theta_ 对齐质心（鲁棒重拟合已生效时即内点质心）。
    double c = cos(theta_), s = sin(theta_);
    tx_ = cx_m - (c * cx_l - s * cy_l);
    ty_ = cy_m - (s * cx_l + c * cy_l);

    aligned_ = true;
}

void GlobalOptimization::UpdateVerticalAlignment_()
{
    if (locked_)
        return;
    std::vector<double> offsets;
    offsets.reserve(window_.size());
    for (const auto &p : window_)
    {
        if (!p.z_valid || !std::isfinite(p.mz) || !std::isfinite(p.lz))
            continue;
        const double dz = p.mz - p.lz;
        if (std::isfinite(dz) && std::fabs(dz) <= vertical_max_abs_offset_)
            offsets.push_back(dz);
    }

    last_vertical_inlier_count_ = 0;
    if (offsets.size() < vertical_min_samples_)
        return;

    std::vector<double> sorted = offsets;
    const size_t middle = sorted.size() / 2;
    std::nth_element(sorted.begin(), sorted.begin() + middle, sorted.end());
    double median = sorted[middle];
    if (sorted.size() % 2 == 0)
    {
        const double lower =
            *std::max_element(sorted.begin(), sorted.begin() + middle);
        median = 0.5 * (lower + median);
    }

    std::vector<double> inliers;
    inliers.reserve(offsets.size());
    for (double dz : offsets)
    {
        if (std::fabs(dz - median) <= vertical_outlier_threshold_)
            inliers.push_back(dz);
    }
    last_vertical_inlier_count_ = inliers.size();
    last_vertical_outlier_ratio_ =
        static_cast<double>(offsets.size() - inliers.size()) /
        static_cast<double>(offsets.size());

    if (inliers.size() < vertical_min_samples_ ||
        inliers.size() * 2 <= offsets.size())
        return;

    double mean = 0.0;
    for (double dz : inliers)
        mean += dz;
    mean /= static_cast<double>(inliers.size());
    double variance = 0.0;
    for (double dz : inliers)
    {
        const double error = dz - mean;
        variance += error * error;
    }
    variance /= static_cast<double>(inliers.size());
    last_vertical_std_ = std::sqrt(std::max(0.0, variance));
    if (!std::isfinite(mean) || !std::isfinite(last_vertical_std_) ||
        std::fabs(mean) > vertical_max_abs_offset_ ||
        last_vertical_std_ > vertical_max_std_)
        return;

    tz_ = mean;
    vertical_aligned_ = true;
}

void GlobalOptimization::RemapLastGlobal_()
{
    if (!hasLastLio_)
        return;
    double c = cos(theta_), s = sin(theta_);
    lastGlobalP_.x() = c * lastLioP_.x() - s * lastLioP_.y() + tx_;
    lastGlobalP_.y() = s * lastLioP_.x() + c * lastLioP_.y() + ty_;
    lastGlobalP_.z() = lastLioP_.z() + tz_;
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

bool GlobalOptimization::Is4DAligned() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return aligned_ && vertical_aligned_;
}

bool GlobalOptimization::IsVerticalAligned() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return vertical_aligned_;
}

bool GlobalOptimization::GetTransform(double &theta, double &tx, double &ty) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    theta = theta_;
    tx = tx_;
    ty = ty_;
    return aligned_;
}

bool GlobalOptimization::GetTransform4D(double &theta, double &tx, double &ty,
                                        double &tz) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    theta = theta_;
    tx = tx_;
    ty = ty_;
    tz = tz_;
    return aligned_ && vertical_aligned_;
}

bool GlobalOptimization::GetSeedOnlyTransform(double &theta, double &tx, double &ty) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (!has_last_gps_ || !hasLastMatchedLio_ || !last_yaw_valid_)
        return false;
    // 与 UpdateAlignment_ 里的 RTK 航向种子分支（global_fusion.cc 内该函数）公式
    // 完全一致，但这里不经滑窗/锁定状态机——每次调用都用最近一组
    // 通过门控的同时GPS/LIO配对重新计算，纯只读、不写任何生产状态（theta_/tx_/ty_/aligned_/locked_
    // 等一概不碰），可与 GetTransform() 的结果并排持续对比。
    theta = WrapPi(last_enu_yaw_ - last_lio_yaw_);
    double c = cos(theta), s = sin(theta);
    tx = last_enu_x_ -
         (c * lastMatchedLioP_.x() - s * lastMatchedLioP_.y());
    ty = last_enu_y_ -
         (s * lastMatchedLioP_.x() + c * lastMatchedLioP_.y());
    return true;
}

bool GlobalOptimization::GetSeedOnlyTransform4D(double &theta, double &tx,
                                                 double &ty, double &tz) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (!has_last_gps_ || !hasLastMatchedLio_ || !last_yaw_valid_ ||
        !last_z_valid_)
        return false;
    theta = WrapPi(last_enu_yaw_ - last_lio_yaw_);
    const double c = cos(theta), s = sin(theta);
    tx = last_enu_x_ -
         (c * lastMatchedLioP_.x() - s * lastMatchedLioP_.y());
    ty = last_enu_y_ -
         (s * lastMatchedLioP_.x() + c * lastMatchedLioP_.y());
    tz = last_enu_z_ - lastMatchedLioP_.z();
    return std::isfinite(tz) && std::fabs(tz) <= vertical_max_abs_offset_;
}

double GlobalOptimization::GetFitResidualRms() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_fit_residual_rms_;
}

double GlobalOptimization::GetOutlierRatio() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_outlier_ratio_;
}

size_t GlobalOptimization::GetHorizontalInlierCount() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_horizontal_inlier_count_;
}

double GlobalOptimization::GetLockConditionNumber() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_lock_condition_number_;
}

double GlobalOptimization::GetAccumulatedTurnRad() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_accumulated_turn_rad_;
}

double GlobalOptimization::GetVerticalStd() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_vertical_std_;
}

double GlobalOptimization::GetVerticalOutlierRatio() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_vertical_outlier_ratio_;
}

size_t GlobalOptimization::GetVerticalInlierCount() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_vertical_inlier_count_;
}

double GlobalOptimization::GetAlongStd() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_along_std_;
}

double GlobalOptimization::GetCrossStd() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_cross_std_;
}

size_t GlobalOptimization::GetWindowSize() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return last_window_n_;
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

void GlobalOptimization::SetRobustParams(int lock_min_samples, int lock_confirm_frames,
                                         double outlier_residual_thresh)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (lock_min_samples >= 2)
        lock_min_samples_ = static_cast<size_t>(lock_min_samples);
    if (lock_confirm_frames >= 1)
        lock_confirm_frames_ = lock_confirm_frames;
    if (outlier_residual_thresh > 0.0)
        outlier_residual_thresh_ = outlier_residual_thresh;
}

void GlobalOptimization::SetTimeAlignmentParams(
    double max_pair_dt_sec, double max_extrapolation_sec)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (std::isfinite(max_pair_dt_sec) && max_pair_dt_sec > 0.0)
        max_lio_pair_dt_sec_ = max_pair_dt_sec;
    if (std::isfinite(max_extrapolation_sec) &&
        max_extrapolation_sec >= 0.0)
        max_lio_extrapolation_sec_ = max_extrapolation_sec;
}

void GlobalOptimization::SetInnovationParams(double unlocked_xy_m,
                                             double locked_xy_m,
                                             double yaw_rad, double z_m)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (std::isfinite(unlocked_xy_m) && unlocked_xy_m > 0.0)
        innovation_unlocked_xy_m_ = unlocked_xy_m;
    if (std::isfinite(locked_xy_m) && locked_xy_m > 0.0)
        innovation_locked_xy_m_ = locked_xy_m;
    if (std::isfinite(yaw_rad) && yaw_rad > 0.0)
        innovation_yaw_rad_ = yaw_rad;
    if (std::isfinite(z_m) && z_m > 0.0)
        innovation_z_m_ = z_m;
}

void GlobalOptimization::SetLockGeometryParams(
    double min_inlier_ratio, double min_along_std_m,
    double max_condition_number,
    double min_accumulated_turn_rad)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (std::isfinite(min_inlier_ratio) && min_inlier_ratio > 0.5 &&
        min_inlier_ratio <= 1.0)
        lock_min_inlier_ratio_ = min_inlier_ratio;
    if (std::isfinite(min_along_std_m) && min_along_std_m > 0.0)
        lock_min_along_std_m_ = min_along_std_m;
    if (std::isfinite(max_condition_number) && max_condition_number >= 1.0)
        lock_max_condition_number_ = max_condition_number;
    if (std::isfinite(min_accumulated_turn_rad) &&
        min_accumulated_turn_rad > 0.0 &&
        min_accumulated_turn_rad <= M_PI * 2.0)
        lock_min_accumulated_turn_rad_ = min_accumulated_turn_rad;
}

void GlobalOptimization::SetLockIndependenceParams(
    double min_interval_sec, double min_displacement_m)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (std::isfinite(min_interval_sec) && min_interval_sec > 0.0)
        lock_confirmation_min_interval_sec_ = min_interval_sec;
    if (std::isfinite(min_displacement_m) && min_displacement_m > 0.0)
        lock_confirmation_min_displacement_m_ = min_displacement_m;
}

void GlobalOptimization::SetVerticalParams(int min_samples, double max_std,
                                           double outlier_threshold,
                                           double max_abs_offset)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (min_samples >= 2)
        vertical_min_samples_ = static_cast<size_t>(min_samples);
    if (std::isfinite(max_std) && max_std > 0.0)
        vertical_max_std_ = max_std;
    if (std::isfinite(outlier_threshold) && outlier_threshold > 0.0)
        vertical_outlier_threshold_ = outlier_threshold;
    if (std::isfinite(max_abs_offset) && max_abs_offset > 0.0)
        vertical_max_abs_offset_ = max_abs_offset;
}

bool GlobalOptimization::RestoreTransform4D(double theta, double tx, double ty,
                                            double tz,
                                            bool horizontal_locked)
{
    std::lock_guard<std::mutex> lk(mtx_);
    constexpr double kMaxHorizontalTranslation = 1.0e7; // 10,000 km local-map sanity bound
    if (!std::isfinite(theta) || !std::isfinite(tx) || !std::isfinite(ty) ||
        !std::isfinite(tz) || std::fabs(tx) > kMaxHorizontalTranslation ||
        std::fabs(ty) > kMaxHorizontalTranslation ||
        std::fabs(tz) > vertical_max_abs_offset_)
        return false;
    theta_ = WrapPi(theta);
    tx_ = tx;
    ty_ = ty;
    tz_ = tz;
    aligned_ = true;
    locked_ = horizontal_locked;
    vertical_aligned_ = true;
    lock_confirm_cnt_ = horizontal_locked ? lock_confirm_frames_ : 0;
    pending_lock_transform_valid_ = false;
    RemapLastGlobal_();
    return true;
}
