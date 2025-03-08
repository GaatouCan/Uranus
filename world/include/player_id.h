#pragma once

#include "common.h"

#if defined(_WIN32) || defined(_WIN64)
#include <xhash>
#elif defined(__linux__)
#include <functional>
#include <cstdint>
#endif

static constexpr int64_t CROSS_SERVER_ID_OFFSET = 1'000'000;
static constexpr int64_t PLAYER_LOCAL_ID_BEGIN = 1'000;
static constexpr int64_t PLAYER_LOCAL_ID_END = 99'999;

struct BASE_API FPlayerID {
    // 1000 - 99999
    int32_t local = 0;
    int32_t cross = 0;

    FPlayerID() = default;

    explicit FPlayerID(const int64_t pid) {
        fromInt64(pid);
    }

    [[nodiscard]] int32_t getLocalID() const {
        return local;
    }

    [[nodiscard]] int32_t getCrossID() const {
        return cross;
    }

    [[nodiscard]] int64_t toInt64() const {
        return cross * CROSS_SERVER_ID_OFFSET + local;
    }

    FPlayerID &fromInt64(const int64_t id) {
        local = static_cast<int32_t>(id % CROSS_SERVER_ID_OFFSET);
        cross = static_cast<int32_t>(id / CROSS_SERVER_ID_OFFSET);
        return *this;
    }

    bool operator<(const FPlayerID &other) const {
        return toInt64() < other.toInt64();
    }

    bool operator==(const FPlayerID& other) const {
        return local == other.local && cross == other.cross;
    }

    bool operator!=(const FPlayerID& other) const {
        return !(*this == other);
    }

    bool operator()(const FPlayerID &lhs, const FPlayerID &rhs) const {
        return lhs.toInt64() < rhs.toInt64();
    }

    [[nodiscard]] bool available() const {
        return local >= PLAYER_LOCAL_ID_BEGIN && local <= PLAYER_LOCAL_ID_END && cross > 0;
    }
};

struct BASE_API FPlayerHash {
    std::size_t operator()(const FPlayerID& pid) const {
        // 使用 std::hash 组合 x 和 y 的哈希值
        return std::hash<uint32_t>()(pid.local) ^ (std::hash<uint32_t>()(pid.cross) << 1);
    }
};