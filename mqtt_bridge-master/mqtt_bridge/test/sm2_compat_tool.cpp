#include <yaml-cpp/yaml.h>

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "mqtt_bridge/sm2_crypto.hpp"

namespace
{

uint8_t hexDigit(char value)
{
  if (value >= '0' && value <= '9') {
    return static_cast<uint8_t>(value - '0');
  }
  value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
  if (value >= 'a' && value <= 'f') {
    return static_cast<uint8_t>(value - 'a' + 10);
  }
  throw std::runtime_error("invalid hexadecimal input");
}

std::string fromHex(const std::string & hex)
{
  if (hex.size() % 2 != 0) {
    throw std::runtime_error("hexadecimal input must have an even length");
  }

  std::string bytes;
  bytes.reserve(hex.size() / 2);
  for (std::size_t index = 0; index < hex.size(); index += 2) {
    bytes.push_back(static_cast<char>((hexDigit(hex[index]) << 4) | hexDigit(hex[index + 1])));
  }
  return bytes;
}

std::string toHex(const std::string & bytes)
{
  std::ostringstream output;
  output << std::hex << std::setfill('0');
  for (const unsigned char byte : bytes) {
    output << std::setw(2) << static_cast<unsigned int>(byte);
  }
  return output.str();
}

}  // namespace

int main(int argc, char ** argv)
{
  if (argc != 3) {
    std::cerr << "usage: sm2_compat_tool encrypt|decrypt HEX_DATA\n";
    return 2;
  }

  try {
    const YAML::Node config = YAML::LoadFile(SM2_TEST_KEY_FILE);
    mqtt_bridge::SM2Crypto crypto(
      config["sm2"]["private_key"].as<std::string>(),
      config["sm2"]["public_key"].as<std::string>());
    const std::string input = fromHex(argv[2]);

    if (std::string(argv[1]) == "encrypt") {
      std::cout << toHex(crypto.encryptC1C2C3(input)) << '\n';
      return 0;
    }
    if (std::string(argv[1]) == "decrypt") {
      std::cout << toHex(crypto.decryptC1C2C3(input)) << '\n';
      return 0;
    }
    throw std::runtime_error("operation must be encrypt or decrypt");
  } catch (const std::exception & error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
