#pragma once

#include "ChatRoom.h"

#include <UniqueID.h>
#include <system/manager/BaseManager.h>

#include <map>

class UChatManager final : public IBaseManager {

    std::map<FUniqueID, UChatRoom *> mChatRoomMap;

public:

    UChatManager();
    ~UChatManager() override;

    void Init() override;

    MANAGER_BODY(UChatManager)
};
