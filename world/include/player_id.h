#pragma once

#include "common.h"

#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
#include <xhash>
#else
#include <functional>
#endif

static constexpr int64_t CROSS_SERVER_ID_OFFSET = 1'000'000;
static constexpr int64_t PLAYER_LOCAL_ID_BEGIN = 1'000;
static constexpr int64_t PLAYER_LOCAL_ID_END = 99'999;

struct BASE_API PlayerID {
    // 1000 - 99999
    int32_t local = 0;
    int32_t cross = 0;

    [[nodiscard]] int32_t GetLocalID() const {
        return local;
    }

    [[nodiscard]] int32_t GetCrossID() const {
        return cross;
    }

    [[nodiscard]] int64_t ToInt64() const {
        return cross * CROSS_SERVER_ID_OFFSET + local;
    }

    PlayerID &FromInt64(const int64_t id) {
        local = static_cast<int32_t>(id % CROSS_SERVER_ID_OFFSET);
        cross = static_cast<int32_t>(id / CROSS_SERVER_ID_OFFSET);
        return *this;
    }

    bool operator<(const PlayerID &other) const {
        return ToInt64() < other.ToInt64();
    }

    bool operator==(const PlayerID& other) const {
        return local == other.local && cross == other.cross;
    }

    bool operator!=(const PlayerID& other) const {
        return !(*this == other);
    }

    bool operator()(const PlayerID &lhs, const PlayerID &rhs) const {
        return lhs.ToInt64() < rhs.ToInt64();
    }

    [[nodiscard]] bool IsAvailable() const {
        return local >= PLAYER_LOCAL_ID_BEGIN && local <= PLAYER_LOCAL_ID_END && cross > 0;
    }
};

struct BASE_API PlayerHash {
    std::size_t operator()(const PlayerID& pid) const {
        // 使用 std::hash 组合 x 和 y 的哈希值
        return std::hash<uint32_t>()(pid.local) ^ (std::hash<uint32_t>()(pid.cross) << 1);
    }
};