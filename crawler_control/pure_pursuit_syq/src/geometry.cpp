#include "geometry.h"

namespace robot {
namespace geometry {
/*处理二维向量（Vec2f 类型）的几何操作
    squaredDistance函数 计算点 p 到线段 se（由起点 s 和终点 e 定义）的平方距离
    latError函数  计算点 p 到线段 se（由起点 s 和终点 e 定义）的垂直距离（也称为侧向误差或横向误差）
*/
float squaredDistance(const Vec2f& p, const Vec2f& s, const Vec2f& e)
{
    Vec2f sp = p - s;
    Vec2f se = e - s;
    if(se.length() < 0.0001f)
        return 0.0f;
    float dot = sp.dot(se);
    if(dot > 0 && dot/se.length() < se.length())
        return fabs(sp.cross(se))/se.length();
    else
        return std::min((s - p).length(), (e - p).length());
}

float latError(const Vec2f& p, const Vec2f& s, const Vec2f& e)
{
    Vec2f sp = p - s;
    Vec2f se = e - s;
    if(se.length() < 0.0001f)
        return 0.0f;
    return fabs(sp.cross(se)/se.length());
}


}  // namespace geometry
}  // namespace robot