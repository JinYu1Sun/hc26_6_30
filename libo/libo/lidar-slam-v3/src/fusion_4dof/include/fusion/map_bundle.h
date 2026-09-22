#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace fusion_4dof {

constexpr uint32_t kMapMetadataSchemaVersion = 1;
constexpr const char *kMapCoordinateFrame = "ENU_V1";

struct MapOrigin {
  int32_t map_index = 0;
  double latitude = 0.0;
  double longitude = 0.0;
  double height = 0.0;
  double gauss_yaw = 0.0;
};

struct MapMetadata {
  uint32_t schema_version = kMapMetadataSchemaVersion;
  std::string map_uuid;
  std::string logical_name;
  std::string coordinate_frame = kMapCoordinateFrame;
  std::string origin_checksum;
  uint64_t created_unix_ns = 0;
};

struct MapBundle {
  MapOrigin origin;
  std::array<uint8_t, 36> origin_bytes{};
  MapMetadata metadata;
};

bool IsSafeLogicalMapName(const std::string &name);
std::string MapMetadataPath(const std::string &origin_path);
std::string OriginChecksum(const std::array<uint8_t, 36> &bytes);
std::string GenerateUuidV4();
bool EncodeMapOrigin(const MapOrigin &origin, std::array<uint8_t, 36> *bytes,
                     std::string *error);
bool ReadMapOrigin(const std::string &path, MapOrigin *origin,
                   std::array<uint8_t, 36> *bytes, std::string *error);
bool ReadMapMetadata(const std::string &path, MapMetadata *metadata,
                     std::string *error);
bool LoadMapBundle(const std::string &origin_path,
                   const std::string &expected_logical_name,
                   bool allow_missing_metadata, MapBundle *bundle,
                   bool *metadata_missing, std::string *error);
bool WriteMapBundleAtomically(const std::string &origin_path,
                              const MapBundle &bundle, bool overwrite,
                              std::string *error);

} // namespace fusion_4dof
