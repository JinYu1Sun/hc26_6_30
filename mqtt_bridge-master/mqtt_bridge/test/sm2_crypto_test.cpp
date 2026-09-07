#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include <string>

#include "mqtt_bridge/sm2_crypto.hpp"

namespace
{

mqtt_bridge::SM2Crypto createCrypto()
{
  const YAML::Node config = YAML::LoadFile(SM2_TEST_KEY_FILE);
  return mqtt_bridge::SM2Crypto(
    config["sm2"]["private_key"].as<std::string>(),
    config["sm2"]["public_key"].as<std::string>());
}

TEST(SM2CryptoTest, BinaryC1C2C3RoundTrip)
{
  auto crypto = createCrypto();
  const std::string plaintext("\xa4\x00\x01\x01\x00\xff\x00\x7f", 8);
  const auto ciphertext = crypto.encryptC1C2C3(plaintext);

  ASSERT_GT(ciphertext.size(), plaintext.size());
  EXPECT_EQ(crypto.decryptC1C2C3(ciphertext), plaintext);
}

TEST(SM2CryptoTest, RejectsTamperedCiphertext)
{
  auto crypto = createCrypto();
  auto ciphertext = crypto.encryptC1C2C3("mqtt-sm2-integrity-test");
  ciphertext.back() ^= 0x01;

  EXPECT_THROW(crypto.decryptC1C2C3(ciphertext), std::runtime_error);
}

}  // namespace
