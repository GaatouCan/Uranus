#pragma once

#include "PlayerID.h"
#include "utils.h"

struct FCacheNode {
    FPlayerID pid;
    ATimePoint lastLoginTime;
    ATimePoint lastLogoutTime;

    [[nodiscard]] bool IsOnline() const {
        constexpr auto zeroPoint = ATimePoint();
        const auto now = NowTimePoint();

        return lastLoginTime > zeroPoint && lastLoginTime <= now
               && (lastLogoutTime > zeroPoint && lastLogoutTime <= now)
               && lastLoginTime > lastLogoutTime;
    }
};
