#ifndef FUSION_4DOF_ALIGNMENT_STATE_H
#define FUSION_4DOF_ALIGNMENT_STATE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

namespace fusion_4dof
{

// Fixed-width, padding-independent codec for same-LIO-generation 4DoF state.
// The byte layout is deliberately explicit so compiler/ABI struct padding can
// never turn a valid state into a different map identity.
struct AlignmentState
{
    uint32_t generation_sec = 0;
    uint32_t generation_nsec = 0;
    int32_t map_index = 0;
    double origin_latitude = 0.0;
    double origin_longitude = 0.0;
    double origin_height = 0.0;
    double origin_yaw = 0.0;
    double theta = 0.0;
    double tx = 0.0;
    double ty = 0.0;
    double tz = 0.0;
    bool horizontal_locked = false;
    std::string map_uuid;
    std::string coordinate_frame;
    std::string origin_checksum;
};

// v2 layout (200 bytes): fixed numeric payload followed by canonical,
// zero-padded map identity strings and a checksum. v1 files are deliberately
// rejected so an old origin-only cache can never bypass the new UUID gate.
using AlignmentStateBytes = std::array<unsigned char, 200>;

inline bool EncodeFixedString(const std::string &value, unsigned char *output,
                              std::size_t capacity)
{
    if (output == nullptr || value.size() >= capacity)
        return false;
    std::memset(output, 0, capacity);
    if (!value.empty())
        std::memcpy(output, value.data(), value.size());
    return true;
}

inline bool DecodeFixedString(const unsigned char *input, std::size_t capacity,
                              std::string *value)
{
    if (input == nullptr || value == nullptr)
        return false;
    std::size_t length = 0;
    while (length < capacity && input[length] != 0)
        ++length;
    if (length == capacity)
        return false;
    for (std::size_t i = length + 1; i < capacity; ++i)
    {
        if (input[i] != 0)
            return false;
    }
    value->assign(reinterpret_cast<const char *>(input), length);
    return true;
}

inline uint64_t AlignmentStateChecksum(const AlignmentStateBytes &data)
{
    uint64_t hash = 14695981039346656037ULL; // FNV-1a 64-bit offset basis
    for (std::size_t i = 0; i < 192; ++i)
    {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= 1099511628211ULL;
    }
    return hash;
}

inline AlignmentStateBytes EncodeAlignmentState(const AlignmentState &state)
{
    static_assert(sizeof(double) == 8, "4DoF state requires 8-byte double");
    AlignmentStateBytes data{};
    const char magic[8] = {'L', 'I', 'O', '4', 'D', 'O', 'F', '2'};
    const uint32_t version = 2;
    std::memcpy(data.data(), magic, 8);
    std::memcpy(data.data() + 8, &version, 4);
    std::memcpy(data.data() + 12, &state.generation_sec, 4);
    std::memcpy(data.data() + 16, &state.generation_nsec, 4);
    std::memcpy(data.data() + 20, &state.map_index, 4);
    std::memcpy(data.data() + 24, &state.origin_latitude, 8);
    std::memcpy(data.data() + 32, &state.origin_longitude, 8);
    std::memcpy(data.data() + 40, &state.origin_height, 8);
    std::memcpy(data.data() + 48, &state.origin_yaw, 8);
    std::memcpy(data.data() + 56, &state.theta, 8);
    std::memcpy(data.data() + 64, &state.tx, 8);
    std::memcpy(data.data() + 72, &state.ty, 8);
    std::memcpy(data.data() + 80, &state.tz, 8);
    data[88] = state.horizontal_locked ? 1U : 0U;
    if (!EncodeFixedString(state.map_uuid, data.data() + 96, 40) ||
        !EncodeFixedString(state.coordinate_frame, data.data() + 136, 24) ||
        !EncodeFixedString(state.origin_checksum, data.data() + 160, 32))
        return AlignmentStateBytes{};
    const uint64_t checksum = AlignmentStateChecksum(data);
    std::memcpy(data.data() + 192, &checksum, 8);
    return data;
}

inline bool DecodeAlignmentState(const AlignmentStateBytes &data,
                                 AlignmentState *state)
{
    if (state == nullptr)
        return false;
    const char magic[8] = {'L', 'I', 'O', '4', 'D', 'O', 'F', '2'};
    uint32_t version = 0;
    uint64_t stored_checksum = 0;
    std::memcpy(&version, data.data() + 8, 4);
    std::memcpy(&stored_checksum, data.data() + 192, 8);
    if (std::memcmp(data.data(), magic, 8) != 0 || version != 2 ||
        stored_checksum != AlignmentStateChecksum(data) || data[88] > 1)
        return false;
    for (std::size_t i = 89; i < 96; ++i)
    {
        if (data[i] != 0)
            return false;
    }

    AlignmentState decoded;
    std::memcpy(&decoded.generation_sec, data.data() + 12, 4);
    std::memcpy(&decoded.generation_nsec, data.data() + 16, 4);
    std::memcpy(&decoded.map_index, data.data() + 20, 4);
    std::memcpy(&decoded.origin_latitude, data.data() + 24, 8);
    std::memcpy(&decoded.origin_longitude, data.data() + 32, 8);
    std::memcpy(&decoded.origin_height, data.data() + 40, 8);
    std::memcpy(&decoded.origin_yaw, data.data() + 48, 8);
    std::memcpy(&decoded.theta, data.data() + 56, 8);
    std::memcpy(&decoded.tx, data.data() + 64, 8);
    std::memcpy(&decoded.ty, data.data() + 72, 8);
    std::memcpy(&decoded.tz, data.data() + 80, 8);
    decoded.horizontal_locked = data[88] != 0;
    if (!DecodeFixedString(data.data() + 96, 40, &decoded.map_uuid) ||
        !DecodeFixedString(data.data() + 136, 24,
                           &decoded.coordinate_frame) ||
        !DecodeFixedString(data.data() + 160, 32,
                           &decoded.origin_checksum))
        return false;
    *state = decoded; // publish only after the complete record was validated
    return true;
}

inline bool SameAlignmentIdentity(const AlignmentState &a,
                                  const AlignmentState &b)
{
    return a.generation_sec == b.generation_sec &&
           a.generation_nsec == b.generation_nsec &&
           a.map_index == b.map_index &&
           a.origin_latitude == b.origin_latitude &&
           a.origin_longitude == b.origin_longitude &&
           a.origin_height == b.origin_height &&
           a.origin_yaw == b.origin_yaw &&
           a.map_uuid == b.map_uuid &&
           a.coordinate_frame == b.coordinate_frame &&
           a.origin_checksum == b.origin_checksum;
}

} // namespace fusion_4dof

#endif // FUSION_4DOF_ALIGNMENT_STATE_H
