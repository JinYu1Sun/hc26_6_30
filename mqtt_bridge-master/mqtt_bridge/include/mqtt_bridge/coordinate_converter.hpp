#ifndef MQTT_BRIDGE__COORDINATE_CONVERTER_HPP_
#define MQTT_BRIDGE__COORDINATE_CONVERTER_HPP_

#include <utility>

namespace mqtt_bridge
{

std::pair<double, double> wgs84ToGcj02(double latitude, double longitude);
std::pair<double, double> gcj02ToWgs84(double latitude, double longitude);

}  // namespace mqtt_bridge

#endif  // MQTT_BRIDGE__COORDINATE_CONVERTER_HPP_
