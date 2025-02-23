#pragma once

#include "player_id.h"
#include "utils.h"

struct CacheNode {
    PlayerID pid;
    TimePoint lastLoginTime;
    TimePoint lastLogoutTime;

    int32_t level = 0;

    int32_t avatar = 0;
    int32_t avatarFrame = 0;

    [[nodiscard]] bool IsOnline() const {
        constexpr auto zeroPoint = TimePoint();
        const auto now = NowTimePoint();

        return lastLoginTime > zeroPoint && lastLoginTime <= now
               && (lastLogoutTime > zeroPoint && lastLogoutTime <= now)
               && lastLoginTime > lastLogoutTime;
    }
};
