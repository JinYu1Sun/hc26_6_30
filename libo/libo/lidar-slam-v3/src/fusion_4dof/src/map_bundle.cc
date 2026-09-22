#include "fusion/map_bundle.h"

#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

namespace fusion_4dof {
namespace {

bool FileExists(const std::string &path) {
  struct stat info;
  return ::stat(path.c_str(), &info) == 0;
}

bool IsUuidV4(const std::string &uuid) {
  if (uuid.size() != 36 || uuid[8] != '-' || uuid[13] != '-' ||
      uuid[18] != '-' || uuid[23] != '-' || uuid[14] != '4' ||
      (uuid[19] != '8' && uuid[19] != '9' && uuid[19] != 'a' &&
       uuid[19] != 'b'))
    return false;
  for (std::size_t i = 0; i < uuid.size(); ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) continue;
    if (!std::isxdigit(static_cast<unsigned char>(uuid[i]))) return false;
  }
  return true;
}

bool WriteBytes(const std::string &path, const uint8_t *data,
                std::size_t size, std::string *error) {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    if (error) *error = "unable to open temporary file: " + path;
    return false;
  }
  output.write(reinterpret_cast<const char *>(data),
               static_cast<std::streamsize>(size));
  output.flush();
  const bool ok = output.good();
  output.close();
  if (!ok || output.fail()) {
    if (error) *error = "unable to completely write temporary file: " + path;
    return false;
  }
  return true;
}

bool WriteText(const std::string &path, const std::string &text,
               std::string *error) {
  return WriteBytes(path, reinterpret_cast<const uint8_t *>(text.data()),
                    text.size(), error);
}

std::string EncodeMetadata(const MapMetadata &metadata) {
  YAML::Emitter emitter;
  emitter << YAML::BeginMap;
  emitter << YAML::Key << "schema_version" << YAML::Value
          << metadata.schema_version;
  emitter << YAML::Key << "map_uuid" << YAML::Value << metadata.map_uuid;
  emitter << YAML::Key << "logical_name" << YAML::Value
          << metadata.logical_name;
  emitter << YAML::Key << "coordinate_frame" << YAML::Value
          << metadata.coordinate_frame;
  emitter << YAML::Key << "origin_checksum" << YAML::Value
          << metadata.origin_checksum;
  emitter << YAML::Key << "created_unix_ns" << YAML::Value
          << metadata.created_unix_ns;
  emitter << YAML::EndMap;
  return std::string(emitter.c_str()) + "\n";
}

void RestoreBackup(const std::string &backup, const std::string &destination,
                   bool existed) {
  std::remove(destination.c_str());
  if (existed) std::rename(backup.c_str(), destination.c_str());
}

} // namespace

bool IsSafeLogicalMapName(const std::string &name) {
  if (name.empty() || name.size() > 96 || name == "." || name == ".." ||
      name == "my_map")
    return false;
  for (char c : name) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (!(std::isalnum(u) || c == '_' || c == '-')) return false;
  }
  return true;
}

std::string MapMetadataPath(const std::string &origin_path) {
  return origin_path + ".meta.yaml";
}

std::string OriginChecksum(const std::array<uint8_t, 36> &bytes) {
  uint64_t hash = 14695981039346656037ULL;
  for (uint8_t byte : bytes) {
    hash ^= byte;
    hash *= 1099511628211ULL;
  }
  std::ostringstream stream;
  stream << "fnv1a64:" << std::hex << std::setfill('0') << std::setw(16)
         << hash;
  return stream.str();
}

std::string GenerateUuidV4() {
  std::array<uint8_t, 16> bytes{};
  try {
    std::random_device random;
    for (uint8_t &byte : bytes) byte = static_cast<uint8_t>(random());
  } catch (const std::exception &) {
    // Some embedded libstdc++ builds expose random_device but have no entropy
    // backend. This fallback retains UUID structure and practical uniqueness;
    // UUID is an identity token, not a cryptographic secret.
    const uint64_t seed = static_cast<uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()) ^
        (static_cast<uint64_t>(::getpid()) << 32U);
    std::mt19937_64 generator(seed);
    for (uint8_t &byte : bytes) byte = static_cast<uint8_t>(generator());
  }
  bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0fU) | 0x40U);
  bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3fU) | 0x80U);
  std::ostringstream out;
  out << std::hex << std::setfill('0');
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    if (i == 4 || i == 6 || i == 8 || i == 10) out << '-';
    out << std::setw(2) << static_cast<unsigned int>(bytes[i]);
  }
  return out.str();
}

bool EncodeMapOrigin(const MapOrigin &origin, std::array<uint8_t, 36> *bytes,
                     std::string *error) {
  if (error) error->clear();
  static_assert(sizeof(int32_t) == 4 && sizeof(double) == 8,
                "map-origin ABI requires int32 and IEEE-754 doubles");
  if (bytes == nullptr) {
    if (error) *error = "null map-origin output";
    return false;
  }
  if (!std::isfinite(origin.latitude) || origin.latitude < -90.0 ||
      origin.latitude > 90.0 || !std::isfinite(origin.longitude) ||
      origin.longitude < -180.0 || origin.longitude > 180.0 ||
      !std::isfinite(origin.height) || !std::isfinite(origin.gauss_yaw) ||
      origin.gauss_yaw < 0.0 || origin.gauss_yaw >= 360.0) {
    if (error) *error = "map-origin values are outside valid ranges";
    return false;
  }
  bytes->fill(0);
  std::memcpy(bytes->data(), &origin.map_index, 4);
  std::memcpy(bytes->data() + 4, &origin.latitude, 8);
  std::memcpy(bytes->data() + 12, &origin.longitude, 8);
  std::memcpy(bytes->data() + 20, &origin.height, 8);
  std::memcpy(bytes->data() + 28, &origin.gauss_yaw, 8);
  return true;
}

bool ReadMapOrigin(const std::string &path, MapOrigin *origin,
                   std::array<uint8_t, 36> *bytes, std::string *error) {
  if (error) error->clear();
  if (origin == nullptr || bytes == nullptr) {
    if (error) *error = "null map bundle output";
    return false;
  }
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    if (error) *error = "unable to open map-origin: " + path;
    return false;
  }
  input.read(reinterpret_cast<char *>(bytes->data()), bytes->size());
  const std::streamsize count = input.gcount();
  char trailing = 0;
  const bool extra = static_cast<bool>(input.read(&trailing, 1));
  if (count != static_cast<std::streamsize>(bytes->size()) || extra) {
    if (error) *error = "map-origin must contain exactly 36 bytes: " + path;
    return false;
  }
  std::memcpy(&origin->map_index, bytes->data(), 4);
  std::memcpy(&origin->latitude, bytes->data() + 4, 8);
  std::memcpy(&origin->longitude, bytes->data() + 12, 8);
  std::memcpy(&origin->height, bytes->data() + 20, 8);
  std::memcpy(&origin->gauss_yaw, bytes->data() + 28, 8);
  std::array<uint8_t, 36> checked{};
  return EncodeMapOrigin(*origin, &checked, error);
}

bool ReadMapMetadata(const std::string &path, MapMetadata *metadata,
                     std::string *error) {
  if (error) error->clear();
  if (metadata == nullptr) return false;
  try {
    const YAML::Node root = YAML::LoadFile(path);
    metadata->schema_version = root["schema_version"].as<uint32_t>();
    metadata->map_uuid = root["map_uuid"].as<std::string>();
    metadata->logical_name = root["logical_name"].as<std::string>();
    metadata->coordinate_frame = root["coordinate_frame"].as<std::string>();
    metadata->origin_checksum = root["origin_checksum"].as<std::string>();
    metadata->created_unix_ns = root["created_unix_ns"].as<uint64_t>();
  } catch (const std::exception &exception) {
    if (error) *error = "invalid map metadata " + path + ": " + exception.what();
    return false;
  }
  if (metadata->schema_version != kMapMetadataSchemaVersion ||
      metadata->coordinate_frame != kMapCoordinateFrame ||
      !IsUuidV4(metadata->map_uuid) ||
      !IsSafeLogicalMapName(metadata->logical_name) ||
      metadata->created_unix_ns == 0) {
    if (error) *error = "unsupported or malformed map metadata: " + path;
    return false;
  }
  return true;
}

bool LoadMapBundle(const std::string &origin_path,
                   const std::string &expected_logical_name,
                   bool allow_missing_metadata, MapBundle *bundle,
                   bool *metadata_missing, std::string *error) {
  if (error) error->clear();
  if (bundle == nullptr) return false;
  if (metadata_missing) *metadata_missing = false;
  if (!ReadMapOrigin(origin_path, &bundle->origin, &bundle->origin_bytes, error))
    return false;
  const std::string metadata_path = MapMetadataPath(origin_path);
  if (!FileExists(metadata_path)) {
    if (!allow_missing_metadata) {
      if (error) *error = "map metadata is missing: " + metadata_path;
      return false;
    }
    if (metadata_missing) *metadata_missing = true;
    return true;
  }
  if (!ReadMapMetadata(metadata_path, &bundle->metadata, error)) return false;
  if (!expected_logical_name.empty() &&
      bundle->metadata.logical_name != expected_logical_name) {
    if (error) *error = "map logical identity does not match requested name";
    return false;
  }
  if (bundle->metadata.origin_checksum != OriginChecksum(bundle->origin_bytes)) {
    if (error) *error = "map-origin checksum does not match metadata";
    return false;
  }
  return true;
}

bool WriteMapBundleAtomically(const std::string &origin_path,
                              const MapBundle &bundle, bool overwrite,
                              std::string *error) {
  if (error) error->clear();
  std::array<uint8_t, 36> encoded{};
  if (!EncodeMapOrigin(bundle.origin, &encoded, error)) return false;
  if (encoded != bundle.origin_bytes ||
      bundle.metadata.origin_checksum != OriginChecksum(encoded) ||
      bundle.metadata.schema_version != kMapMetadataSchemaVersion ||
      bundle.metadata.coordinate_frame != kMapCoordinateFrame ||
      !IsUuidV4(bundle.metadata.map_uuid) ||
      !IsSafeLogicalMapName(bundle.metadata.logical_name) ||
      bundle.metadata.created_unix_ns == 0) {
    if (error) *error = "refuse internally inconsistent map bundle";
    return false;
  }
  const std::string metadata_path = MapMetadataPath(origin_path);
  const bool origin_existed = FileExists(origin_path);
  const bool metadata_existed = FileExists(metadata_path);
  if (!overwrite && (origin_existed || metadata_existed)) {
    if (error) *error = "map already exists; overwrite was not requested";
    return false;
  }
  const std::string suffix = ".txn." + std::to_string(::getpid());
  const std::string origin_tmp = origin_path + suffix;
  const std::string metadata_tmp = metadata_path + suffix;
  const std::string origin_backup = origin_path + suffix + ".bak";
  const std::string metadata_backup = metadata_path + suffix + ".bak";
  std::remove(origin_tmp.c_str());
  std::remove(metadata_tmp.c_str());
  std::remove(origin_backup.c_str());
  std::remove(metadata_backup.c_str());
  if (!WriteBytes(origin_tmp, encoded.data(), encoded.size(), error) ||
      !WriteText(metadata_tmp, EncodeMetadata(bundle.metadata), error)) {
    std::remove(origin_tmp.c_str());
    std::remove(metadata_tmp.c_str());
    return false;
  }
  if (origin_existed && std::rename(origin_path.c_str(), origin_backup.c_str()) != 0) {
    if (error) *error = "unable to stage previous map-origin for transaction";
    std::remove(origin_tmp.c_str()); std::remove(metadata_tmp.c_str());
    return false;
  }
  if (metadata_existed &&
      std::rename(metadata_path.c_str(), metadata_backup.c_str()) != 0) {
    if (origin_existed) std::rename(origin_backup.c_str(), origin_path.c_str());
    if (error) *error = "unable to stage previous map metadata for transaction";
    std::remove(origin_tmp.c_str()); std::remove(metadata_tmp.c_str());
    return false;
  }
  if (std::rename(origin_tmp.c_str(), origin_path.c_str()) != 0 ||
      std::rename(metadata_tmp.c_str(), metadata_path.c_str()) != 0) {
    RestoreBackup(origin_backup, origin_path, origin_existed);
    RestoreBackup(metadata_backup, metadata_path, metadata_existed);
    std::remove(origin_tmp.c_str()); std::remove(metadata_tmp.c_str());
    if (error) *error = "map bundle commit failed and previous bundle was restored";
    return false;
  }
  std::remove(origin_backup.c_str());
  std::remove(metadata_backup.c_str());
  return true;
}

} // namespace fusion_4dof
