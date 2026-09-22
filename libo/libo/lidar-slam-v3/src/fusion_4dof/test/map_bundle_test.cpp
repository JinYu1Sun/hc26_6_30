#include "fusion/map_bundle.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <unistd.h>

int main() {
  using namespace fusion_4dof;
  assert(IsSafeLogicalMapName("yard_01"));
  assert(!IsSafeLogicalMapName("../yard"));
  assert(!IsSafeLogicalMapName("my_map"));

  MapBundle bundle;
  bundle.origin.map_index = 7;
  bundle.origin.latitude = 31.234;
  bundle.origin.longitude = 121.456;
  bundle.origin.height = 4.2;
  bundle.origin.gauss_yaw = 123.0;
  std::string error;
  assert(EncodeMapOrigin(bundle.origin, &bundle.origin_bytes, &error));
  bundle.metadata.map_uuid = GenerateUuidV4();
  bundle.metadata.logical_name = "yard_01";
  bundle.metadata.origin_checksum = OriginChecksum(bundle.origin_bytes);
  bundle.metadata.created_unix_ns = 123456789;

  const std::string path = "/tmp/fusion_map_bundle_test_" +
                           std::to_string(::getpid()) + ".mp";
  std::remove(path.c_str());
  std::remove(MapMetadataPath(path).c_str());
  assert(WriteMapBundleAtomically(path, bundle, false, &error));

  MapBundle loaded;
  bool missing = true;
  assert(LoadMapBundle(path, "yard_01", false, &loaded, &missing, &error));
  assert(!missing);
  assert(loaded.origin_bytes == bundle.origin_bytes);
  assert(loaded.metadata.map_uuid == bundle.metadata.map_uuid);
  assert(!LoadMapBundle(path, "another_yard", false, &loaded, &missing,
                        &error));
  assert(!WriteMapBundleAtomically(path, bundle, false, &error));

  // Corruption must be detected rather than silently selecting the wrong map.
  std::fstream corrupt(path, std::ios::binary | std::ios::in | std::ios::out);
  char byte = 0;
  corrupt.read(&byte, 1);
  byte ^= 0x01;
  corrupt.seekp(0);
  corrupt.write(&byte, 1);
  corrupt.close();
  assert(!LoadMapBundle(path, "yard_01", false, &loaded, &missing, &error));

  assert(WriteMapBundleAtomically(path, bundle, true, &error));
  assert(LoadMapBundle(path, "yard_01", false, &loaded, &missing, &error));

  // Reusing the canonical .mp path for a new mapping origin must replace both
  // origin and metadata. A stale old checksum must never survive the overwrite.
  MapBundle replacement = bundle;
  replacement.origin.latitude += 0.001;
  assert(EncodeMapOrigin(replacement.origin, &replacement.origin_bytes, &error));
  replacement.metadata.map_uuid = GenerateUuidV4();
  replacement.metadata.logical_name = "unsaved_mapping";
  replacement.metadata.origin_checksum = OriginChecksum(replacement.origin_bytes);
  replacement.metadata.created_unix_ns++;
  assert(WriteMapBundleAtomically(path, replacement, true, &error));
  assert(LoadMapBundle(path, "unsaved_mapping", false, &loaded, &missing,
                       &error));
  assert(loaded.origin_bytes == replacement.origin_bytes);
  assert(loaded.metadata.map_uuid == replacement.metadata.map_uuid);
  assert(loaded.metadata.origin_checksum ==
         OriginChecksum(loaded.origin_bytes));
  std::remove(path.c_str());
  std::remove(MapMetadataPath(path).c_str());
  return 0;
}
