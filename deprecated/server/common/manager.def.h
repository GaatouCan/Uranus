#pragma once

#include <system/manager/ManagerSystem.h>

#include "../player/PlayerManager.h"
#include "../world/manager/chat/ChatManager.h"

inline void LoadManager(UManagerSystem *sys) {
    if (sys == nullptr)
        return;

    sys->CreateManager<UPlayerManager>();
    sys->CreateManager<UChatManager>();
}
