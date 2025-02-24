#pragma once

#include "utils.h"

struct CacheNode {
    int64_t pid;
    int64_t lastLoginTime;
    int64_t lastLogoutTime;

    int32_t level;

    int32_t avatar;
    int32_t avatarFrame;

    [[nodiscard]] bool IsOnline() const {
        const auto now = utils::UnixTime();

        return lastLoginTime > 0 && lastLoginTime <= now
               && (lastLogoutTime > 0 && lastLogoutTime <= now)
               && lastLoginTime > lastLogoutTime;
    }
};
