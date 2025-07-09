#include "player_cache.h"

#include <utils.h>

bool FPlayerCache::IsOnline() const {
    const auto now = utils::UnixTime();
    return loginTime > 0 && logoutTime < loginTime && ((now - syncTime) < 30);
}
