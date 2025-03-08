#pragma once

#define DISPATCH_EVENT(event, param) \
    if (const auto sys = getWorld()->getSystem<UEventSystem>(); sys != nullptr) { \
        sys->dispatch(static_cast<unsigned int>(EEvent::event), (param)); \
    } else { \
        spdlog::critical("{} - Failed to get event system.", __FUNCTION__); \
        delete (param); \
        getWorld()->shutdown(); \
    }


enum class EEvent : unsigned int {
    UNAVAILABLE,
    PLAYER_LOGIN,
    PLAYER_LOGOUT
};
