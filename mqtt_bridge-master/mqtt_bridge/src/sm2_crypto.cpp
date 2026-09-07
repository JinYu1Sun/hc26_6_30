#include "mqtt_bridge/sm2_crypto.hpp"

#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/params.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

constexpr size_t SM2_COORDINATE_SIZE = 32;
constexpr size_t SM3_DIGEST_SIZE = 32;
constexpr size_t SM2_RAW_OVERHEAD = 2 * SM2_COORDINATE_SIZE + SM3_DIGEST_SIZE;
constexpr size_t MAX_SM2_MESSAGE_SIZE = 1024 * 1024;

typedef struct Sm2CiphertextValue_st
{
  ASN1_INTEGER* x_coordinate;
  ASN1_INTEGER* y_coordinate;
  ASN1_OCTET_STRING* hash;
  ASN1_OCTET_STRING* ciphertext;
} Sm2CiphertextValue;

ASN1_SEQUENCE(Sm2CiphertextValue) = {
  ASN1_SIMPLE(Sm2CiphertextValue, x_coordinate, ASN1_INTEGER),
  ASN1_SIMPLE(Sm2CiphertextValue, y_coordinate, ASN1_INTEGER),
  ASN1_SIMPLE(Sm2CiphertextValue, hash, ASN1_OCTET_STRING),
  ASN1_SIMPLE(Sm2CiphertextValue, ciphertext, ASN1_OCTET_STRING)
} ASN1_SEQUENCE_END(Sm2CiphertextValue)

IMPLEMENT_ASN1_FUNCTIONS(Sm2CiphertextValue)

template<typename T, void (*FreeFunction)(T*)>
struct OpenSSLDeleter
{
  void operator()(T* value) const
  {
    if (value != nullptr) {
      FreeFunction(value);
    }
  }
};

using BignumPtr = std::unique_ptr<BIGNUM, OpenSSLDeleter<BIGNUM, BN_free>>;
using PkeyPtr = std::unique_ptr<EVP_PKEY, OpenSSLDeleter<EVP_PKEY, EVP_PKEY_free>>;
using PkeyContextPtr =
  std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter<EVP_PKEY_CTX, EVP_PKEY_CTX_free>>;
using ParamBuilderPtr =
  std::unique_ptr<OSSL_PARAM_BLD, OpenSSLDeleter<OSSL_PARAM_BLD, OSSL_PARAM_BLD_free>>;
using ParamPtr = std::unique_ptr<OSSL_PARAM, OpenSSLDeleter<OSSL_PARAM, OSSL_PARAM_free>>;
using CiphertextValuePtr = std::unique_ptr<
  Sm2CiphertextValue, OpenSSLDeleter<Sm2CiphertextValue, Sm2CiphertextValue_free>>;

std::string opensslError(const std::string& prefix)
{
  std::string message = prefix;
  unsigned long error_code = 0;
  while ((error_code = ERR_get_error()) != 0) {
    char buffer[256];
    ERR_error_string_n(error_code, buffer, sizeof(buffer));
    message += ": ";
    message += buffer;
  }
  return message;
}

uint8_t hexNibble(char value)
{
  if (value >= '0' && value <= '9') {
    return static_cast<uint8_t>(value - '0');
  }
  value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
  if (value >= 'a' && value <= 'f') {
    return static_cast<uint8_t>(value - 'a' + 10);
  }
  throw std::invalid_argument("SM2密钥包含非十六进制字符");
}

std::vector<uint8_t> hexToBytes(const std::string& hex)
{
  if (hex.empty() || hex.size() % 2 != 0) {
    throw std::invalid_argument("SM2密钥十六进制长度无效");
  }

  std::vector<uint8_t> bytes(hex.size() / 2);
  for (size_t i = 0; i < bytes.size(); ++i) {
    bytes[i] = static_cast<uint8_t>((hexNibble(hex[i * 2]) << 4) |
      hexNibble(hex[i * 2 + 1]));
  }
  return bytes;
}

PkeyPtr importKeyPair(
  const std::string& private_key_hex,
  const std::string& public_key_hex)
{
  auto private_key = hexToBytes(private_key_hex);
  auto public_key = hexToBytes(public_key_hex);

  if (private_key.size() != SM2_COORDINATE_SIZE) {
    throw std::invalid_argument("SM2私钥必须为32字节");
  }
  if (public_key.size() == 2 * SM2_COORDINATE_SIZE) {
    public_key.insert(public_key.begin(), 0x04);
  }
  if (public_key.size() != 1 + 2 * SM2_COORDINATE_SIZE || public_key[0] != 0x04) {
    throw std::invalid_argument("SM2公钥必须为04开头的65字节非压缩点");
  }

  BignumPtr private_bn(BN_bin2bn(
    private_key.data(), static_cast<int>(private_key.size()), nullptr));
  if (!private_bn) {
    throw std::runtime_error(opensslError("创建SM2私钥失败"));
  }

  PkeyContextPtr import_context(EVP_PKEY_CTX_new_from_name(nullptr, "SM2", nullptr));
  ParamBuilderPtr builder(OSSL_PARAM_BLD_new());
  if (!import_context || !builder) {
    throw std::runtime_error(opensslError("创建SM2密钥导入上下文失败"));
  }

  if (OSSL_PARAM_BLD_push_utf8_string(
        builder.get(), OSSL_PKEY_PARAM_GROUP_NAME, const_cast<char*>("SM2"), 0) != 1 ||
      OSSL_PARAM_BLD_push_octet_string(
        builder.get(), OSSL_PKEY_PARAM_PUB_KEY, public_key.data(), public_key.size()) != 1 ||
      OSSL_PARAM_BLD_push_BN(
        builder.get(), OSSL_PKEY_PARAM_PRIV_KEY, private_bn.get()) != 1) {
    throw std::runtime_error(opensslError("构建SM2密钥参数失败"));
  }

  ParamPtr parameters(OSSL_PARAM_BLD_to_param(builder.get()));
  if (!parameters || EVP_PKEY_fromdata_init(import_context.get()) != 1) {
    throw std::runtime_error(opensslError("初始化SM2密钥导入失败"));
  }

  EVP_PKEY* raw_key = nullptr;
  if (EVP_PKEY_fromdata(
        import_context.get(), &raw_key, EVP_PKEY_KEYPAIR, parameters.get()) != 1) {
    throw std::runtime_error(opensslError("导入SM2密钥对失败"));
  }
  PkeyPtr key(raw_key);

  PkeyContextPtr check_context(EVP_PKEY_CTX_new(key.get(), nullptr));
  if (!check_context || EVP_PKEY_pairwise_check(check_context.get()) != 1) {
    throw std::invalid_argument(opensslError("SM2公钥与私钥不匹配"));
  }

  return key;
}

void appendCoordinate(std::string& output, const ASN1_INTEGER* coordinate)
{
  BignumPtr value(ASN1_INTEGER_to_BN(coordinate, nullptr));
  if (!value) {
    throw std::runtime_error(opensslError("解析SM2坐标失败"));
  }

  const size_t old_size = output.size();
  output.resize(old_size + SM2_COORDINATE_SIZE);
  if (BN_bn2binpad(
        value.get(),
        reinterpret_cast<unsigned char*>(output.data() + old_size),
        static_cast<int>(SM2_COORDINATE_SIZE)) !=
      static_cast<int>(SM2_COORDINATE_SIZE)) {
    throw std::runtime_error(opensslError("编码SM2坐标失败"));
  }
}

}  // namespace

namespace mqtt_bridge
{

struct SM2Crypto::Impl
{
  PkeyPtr key;
};

SM2Crypto::SM2Crypto(
  const std::string& private_key_hex,
  const std::string& public_key_hex)
  : impl_(std::make_unique<Impl>())
{
  impl_->key = importKeyPair(private_key_hex, public_key_hex);
}

SM2Crypto::~SM2Crypto() = default;

std::string SM2Crypto::encryptC1C2C3(const std::string& plaintext) const
{
  if (plaintext.empty() || plaintext.size() > MAX_SM2_MESSAGE_SIZE) {
    throw std::invalid_argument("SM2明文长度无效");
  }

  PkeyContextPtr context(EVP_PKEY_CTX_new(impl_->key.get(), nullptr));
  if (!context || EVP_PKEY_encrypt_init(context.get()) != 1) {
    throw std::runtime_error(opensslError("初始化SM2加密失败"));
  }

  size_t der_size = 0;
  const auto* input = reinterpret_cast<const unsigned char*>(plaintext.data());
  if (EVP_PKEY_encrypt(context.get(), nullptr, &der_size, input, plaintext.size()) != 1) {
    throw std::runtime_error(opensslError("计算SM2密文长度失败"));
  }

  std::vector<unsigned char> der(der_size);
  if (EVP_PKEY_encrypt(
        context.get(), der.data(), &der_size, input, plaintext.size()) != 1) {
    throw std::runtime_error(opensslError("SM2加密失败"));
  }
  der.resize(der_size);

  const unsigned char* der_cursor = der.data();
  CiphertextValuePtr value(d2i_Sm2CiphertextValue(
    nullptr, &der_cursor, static_cast<long>(der.size())));
  if (!value || der_cursor != der.data() + der.size()) {
    throw std::runtime_error(opensslError("解析OpenSSL SM2密文失败"));
  }
  if (ASN1_STRING_length(value->hash) != static_cast<int>(SM3_DIGEST_SIZE)) {
    throw std::runtime_error("OpenSSL SM2密文中的SM3摘要长度无效");
  }

  std::string output;
  output.reserve(SM2_RAW_OVERHEAD + plaintext.size());
  appendCoordinate(output, value->x_coordinate);
  appendCoordinate(output, value->y_coordinate);
  output.append(
    reinterpret_cast<const char*>(ASN1_STRING_get0_data(value->ciphertext)),
    static_cast<size_t>(ASN1_STRING_length(value->ciphertext)));
  output.append(
    reinterpret_cast<const char*>(ASN1_STRING_get0_data(value->hash)),
    SM3_DIGEST_SIZE);
  return output;
}

std::string SM2Crypto::decryptC1C2C3(const std::string& ciphertext) const
{
  if (ciphertext.size() <= SM2_RAW_OVERHEAD ||
      ciphertext.size() > MAX_SM2_MESSAGE_SIZE + SM2_RAW_OVERHEAD) {
    throw std::invalid_argument("SM2 C1C2C3密文长度无效");
  }

  const auto* input = reinterpret_cast<const unsigned char*>(ciphertext.data());
  BignumPtr x(BN_bin2bn(input, static_cast<int>(SM2_COORDINATE_SIZE), nullptr));
  BignumPtr y(BN_bin2bn(
    input + SM2_COORDINATE_SIZE,
    static_cast<int>(SM2_COORDINATE_SIZE), nullptr));
  if (!x || !y) {
    throw std::runtime_error(opensslError("解析SM2 C1坐标失败"));
  }

  CiphertextValuePtr value(Sm2CiphertextValue_new());
  if (!value ||
      BN_to_ASN1_INTEGER(x.get(), value->x_coordinate) == nullptr ||
      BN_to_ASN1_INTEGER(y.get(), value->y_coordinate) == nullptr) {
    throw std::runtime_error(opensslError("构建SM2 ASN.1坐标失败"));
  }

  const size_t encrypted_data_size = ciphertext.size() - SM2_RAW_OVERHEAD;
  const unsigned char* encrypted_data = input + 2 * SM2_COORDINATE_SIZE;
  const unsigned char* digest = encrypted_data + encrypted_data_size;
  if (ASN1_OCTET_STRING_set(
        value->hash, digest, static_cast<int>(SM3_DIGEST_SIZE)) != 1 ||
      ASN1_OCTET_STRING_set(
        value->ciphertext, encrypted_data,
        static_cast<int>(encrypted_data_size)) != 1) {
    throw std::runtime_error(opensslError("构建SM2 ASN.1密文失败"));
  }

  const int der_size = i2d_Sm2CiphertextValue(value.get(), nullptr);
  if (der_size <= 0) {
    throw std::runtime_error(opensslError("计算SM2 ASN.1密文长度失败"));
  }
  std::vector<unsigned char> der(static_cast<size_t>(der_size));
  unsigned char* der_cursor = der.data();
  if (i2d_Sm2CiphertextValue(value.get(), &der_cursor) != der_size) {
    throw std::runtime_error(opensslError("编码SM2 ASN.1密文失败"));
  }

  PkeyContextPtr context(EVP_PKEY_CTX_new(impl_->key.get(), nullptr));
  if (!context || EVP_PKEY_decrypt_init(context.get()) != 1) {
    throw std::runtime_error(opensslError("初始化SM2解密失败"));
  }

  size_t plaintext_size = 0;
  if (EVP_PKEY_decrypt(
        context.get(), nullptr, &plaintext_size, der.data(), der.size()) != 1) {
    throw std::runtime_error(opensslError("计算SM2明文长度失败"));
  }

  std::string plaintext(plaintext_size, '\0');
  if (EVP_PKEY_decrypt(
        context.get(), reinterpret_cast<unsigned char*>(plaintext.data()),
        &plaintext_size, der.data(), der.size()) != 1) {
    throw std::runtime_error(opensslError("SM2解密或完整性校验失败"));
  }
  plaintext.resize(plaintext_size);
  return plaintext;
}

}  // namespace mqtt_bridge
