#include "fusion/alignment_state.h"

#include <cassert>
#include <cstring>
#include <iostream>

int main()
{
    fusion_4dof::AlignmentState expected;
    expected.generation_sec = 123456U;
    expected.generation_nsec = 789U;
    expected.map_index = 7;
    expected.origin_latitude = 31.123456789;
    expected.origin_longitude = 121.987654321;
    expected.origin_height = 12.5;
    expected.origin_yaw = 73.25;
    expected.theta = -0.31;
    expected.tx = 5.2;
    expected.ty = -7.4;
    expected.tz = 1.8;
    expected.horizontal_locked = true;
    expected.map_uuid = "550e8400-e29b-4d4a-8c7b-123456789abc";
    expected.coordinate_frame = "ENU_V1";
    expected.origin_checksum = "fnv1a64:0123456789abcdef";

    const fusion_4dof::AlignmentStateBytes encoded =
        fusion_4dof::EncodeAlignmentState(expected);
    fusion_4dof::AlignmentState decoded;
    assert(fusion_4dof::DecodeAlignmentState(encoded, &decoded));
    assert(fusion_4dof::SameAlignmentIdentity(expected, decoded));
    assert(decoded.theta == expected.theta && decoded.tx == expected.tx &&
           decoded.ty == expected.ty && decoded.tz == expected.tz &&
           decoded.horizontal_locked == expected.horizontal_locked);

    auto corrupted = encoded;
    corrupted[72] ^= 0x40U;
    assert(!fusion_4dof::DecodeAlignmentState(corrupted, &decoded));

    // Reserved bytes and a non-canonical bool remain invalid even if their
    // checksum is recomputed.
    auto noncanonical = encoded;
    noncanonical[89] = 1U;
    uint64_t checksum = fusion_4dof::AlignmentStateChecksum(noncanonical);
    std::memcpy(noncanonical.data() + 192, &checksum, 8);
    assert(!fusion_4dof::DecodeAlignmentState(noncanonical, &decoded));
    noncanonical = encoded;
    noncanonical[88] = 2U;
    checksum = fusion_4dof::AlignmentStateChecksum(noncanonical);
    std::memcpy(noncanonical.data() + 192, &checksum, 8);
    assert(!fusion_4dof::DecodeAlignmentState(noncanonical, &decoded));
    noncanonical = encoded;
    // Bytes after the first NUL in a fixed identity field must remain zero.
    noncanonical[96 + expected.map_uuid.size() + 1] = 1U;
    checksum = fusion_4dof::AlignmentStateChecksum(noncanonical);
    std::memcpy(noncanonical.data() + 192, &checksum, 8);
    assert(!fusion_4dof::DecodeAlignmentState(noncanonical, &decoded));

    auto legacy_v1 = encoded;
    legacy_v1[7] = '1';
    const uint32_t old_version = 1;
    std::memcpy(legacy_v1.data() + 8, &old_version, 4);
    checksum = fusion_4dof::AlignmentStateChecksum(legacy_v1);
    std::memcpy(legacy_v1.data() + 192, &checksum, 8);
    assert(!fusion_4dof::DecodeAlignmentState(legacy_v1, &decoded));

    auto another_generation = expected;
    ++another_generation.generation_nsec;
    assert(!fusion_4dof::SameAlignmentIdentity(expected, another_generation));
    auto another_map = expected;
    another_map.origin_height += 0.01;
    assert(!fusion_4dof::SameAlignmentIdentity(expected, another_map));
    another_map = expected;
    another_map.map_uuid[35] = 'd';
    assert(!fusion_4dof::SameAlignmentIdentity(expected, another_map));
    another_map = expected;
    another_map.coordinate_frame = "ENU_V2";
    assert(!fusion_4dof::SameAlignmentIdentity(expected, another_map));
    another_map = expected;
    another_map.origin_checksum = "fnv1a64:fedcba9876543210";
    assert(!fusion_4dof::SameAlignmentIdentity(expected, another_map));

    auto oversized = expected;
    oversized.map_uuid.assign(40, 'x');
    assert(!fusion_4dof::DecodeAlignmentState(
        fusion_4dof::EncodeAlignmentState(oversized), &decoded));

    std::cout << "alignment_state_test: PASS" << std::endl;
    return 0;
}
