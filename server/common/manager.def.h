#pragma once

#include "system/manager/manager_system.h"

#include "../player/player_manager.h"
// #include "../world/manager/chat/ChatManager.h"

inline void RegisterManager(ManagerSystem *sys) {
    if (sys == nullptr)
        return;

    sys->CreateManager<PlayerManager>();
    // sys->CreateManager<UChatManager>();
}
