#ifndef MQTT_BRIDGE__SM2_CRYPTO_HPP_
#define MQTT_BRIDGE__SM2_CRYPTO_HPP_

#include <memory>
#include <string>

namespace mqtt_bridge
{

class SM2Crypto
{
public:
  SM2Crypto(const std::string& private_key_hex, const std::string& public_key_hex);
  ~SM2Crypto();

  SM2Crypto(const SM2Crypto&) = delete;
  SM2Crypto& operator=(const SM2Crypto&) = delete;

  std::string encryptC1C2C3(const std::string& plaintext) const;
  std::string decryptC1C2C3(const std::string& ciphertext) const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace mqtt_bridge

#endif  // MQTT_BRIDGE__SM2_CRYPTO_HPP_
