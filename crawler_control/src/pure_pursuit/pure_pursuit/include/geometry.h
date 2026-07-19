#pragma once
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <limits>
#include <iostream>
#include <deque>
#include <vector>

namespace robot {
namespace geometry {

using namespace std;

template <class T>
/**
 * @brief clamp   Set v between the maximum and minimum value
 * @param v
 * @param lo    Minimum value for v.
 * @param hi    Maximum value for v.
 * @return      Return new value of v.
 */
T clamp(const T& v, const T& lo, const T& hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

template <typename T>
/**
 * @brief sqr   Caculate the square of x. 
 * @param x
 * @return    Return the square of x.
 */
T sqr(const T& x) {
  return x * x;
}

template <typename T>
/**求斜边，适用车身坐标系求点距
 * @brief hypotFast    Compute the hypotenuse of a right-angled triangle
 * @param x
 * @param y
 * @return
 */
T hypotFast(T x, T y) {
  return sqrt(sqr(x) + sqr(y));
}

template <typename T>
T NormalizeAngle(const T angle) {
  T a = std::fmod(angle + M_PI, 2.0 * M_PI);
  if (a < 0.0) {
    a += (2.0 * M_PI);
  }
  return a - M_PI;
}

template <typename T>
struct Vec2 {
  T x, y;

  Vec2() : x(0), y(0) {}

  Vec2(T x, T y) : x(x), y(y) {}

  /**
   * @brief operator +    Operator overloading
   * @param other    Another vector
   * @return
   */

  Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }

  Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }

  Vec2 operator*(T scale) const { return {x * scale, y * scale}; }

  Vec2 operator/(T scale) const { return {x / scale, y / scale}; }

  bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }

  /**
   * @brief heading  Get the direction of the vector.
   * @return
   */
  T heading() const { return std::atan2(y, x); }

  /**
   * @brief length   Get the length of the vector.
   * @return
   */
  T length() const { return hypotFast<T>(x, y); }

  /**
   * @brief cross  Multiplication cross of two vectors.
   * @param other  Another vector.
   * @return 结果的正负表示了other相对于*this的旋转方向（逆时针为正）
   */
  T cross(const Vec2& other) const { return x * other.y - y * other.x; }
  
  /**
   * @brief dot   Dot product between two vectors.
   * @param other
   * @return
   */
  T dot(const Vec2& other) const { return x * other.x + y * other.y; }

  /**
   * @brief distance   Caculate the distance between two points.
   * @param other
   * @return
   */
  T distance(const Vec2& other) const { return hypotFast<T>(x - other.x, y - other.y); }

  /**
   * @brief normalize   Get the unit vector
   * @return
   */
  Vec2 normalize() const { return *this / length(); }

};

using Vec2f = Vec2<double>;
using Vec2d = Vec2<double>;


template <typename T>
struct Vec3{
    T x, y, z;
};
using Vec3f = Vec3<float>;
using Vec3d = Vec3<float>;

struct PathPoint{
    Vec2f position;
   // float theta;
    float curvature;  //曲率
    float speed;      //期望速度
    float s;    //离第一个点的路径长度（单位：m）
    float t;
};

struct RobotState{  //实时定位信息
    Vec2f position;
    float yaw;
    float roll;
    float pitch;
    float linearSpeed;
    float goalS;   //目标点s值
};
//定位信息
struct PoseC{
    double x;
    double y;
    float z;
    float roll;
    float yaw;
    float pitch;
    float vel;
};

using Path = std::deque<PathPoint>;


} // namespace geometry
} // namespace robot


namespace robot {
namespace geometry {

float squaredDistance(const Vec2f& p, const Vec2f& s, const Vec2f& e);
float latError(const Vec2f& p, const Vec2f& s, const Vec2f& e);
} // namespace geometry
} // namespace robot
