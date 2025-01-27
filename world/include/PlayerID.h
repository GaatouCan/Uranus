#pragma once

#include "common.h"

#include <cstdint>
#include <xhash>

static constexpr unsigned int kCrossServerIDOffset = 1'000'000;
static constexpr unsigned int kPlayerLocalIDBegin = 1'000;
static constexpr unsigned int kPlayerLocalIDEnd = 99'999;

struct BASE_API FPlayerID {
    // 1000 - 99999
    uint32_t localID = 0;
    uint32_t crossID = 0;

    [[nodiscard]] uint32_t GetLocalID() const {
        return localID;
    }

    [[nodiscard]] uint32_t GetCrossID() const {
        return crossID;
    }

    [[nodiscard]] uint64_t ToUInt64() const {
        return crossID * kCrossServerIDOffset + localID;
    }

    FPlayerID &FromUInt64(const uint64_t id) {
        localID = id % kCrossServerIDOffset;
        crossID = id / kCrossServerIDOffset;
        return *this;
    }

    bool operator<(const FPlayerID &other) const {
        return ToUInt64() < other.ToUInt64();
    }

    bool operator==(const FPlayerID& other) const {
        return localID == other.localID && crossID == other.crossID;
    }

    bool operator!=(const FPlayerID& other) const {
        return !(*this == other);
    }

    bool operator()(const FPlayerID &lhs, const FPlayerID &rhs) const {
        return lhs.ToUInt64() < rhs.ToUInt64();
    }

    [[nodiscard]] bool IsAvailable() const {
        return localID >= kPlayerLocalIDBegin && localID <= kPlayerLocalIDEnd && crossID > 0;
    }
};

struct BASE_API FPlayerHash {
    std::size_t operator()(const FPlayerID& pid) const {
        // 使用 std::hash 组合 x 和 y 的哈希值
        return std::hash<uint32_t>()(pid.localID) ^ (std::hash<uint32_t>()(pid.crossID) << 1);
    }
};