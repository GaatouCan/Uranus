#pragma once

#include "system/manager/manager_system.h"

#include "../player/player_manager.h"
// #include "../world/manager/chat/ChatManager.h"

inline void RegisterManager(UManagerSystem *sys) {
    if (sys == nullptr)
        return;

    REGISTER_MANAGER(PlayerManager)
    // sys->CreateManager<UChatManager>();
}
