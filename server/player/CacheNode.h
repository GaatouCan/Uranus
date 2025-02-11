#pragma once

#include "PlayerID.h"
#include "utils.h"

struct FCacheNode {
    FPlayerID pid;
    TimePoint lastLoginTime;
    TimePoint lastLogoutTime;

    [[nodiscard]] bool IsOnline() const {
        constexpr auto zeroPoint = TimePoint();
        const auto now = NowTimePoint();

        return lastLoginTime > zeroPoint && lastLoginTime <= now
               && (lastLogoutTime > zeroPoint && lastLogoutTime <= now)
               && lastLoginTime > lastLogoutTime;
    }
};
