#include "ChatManager.h"
#include "../../../player/Player.h"
#include "../../../common/proto.def.h"

#include <system/manager/ManagerSystem.h>
#include <GameWorld.h>

#include <spdlog/spdlog.h>

UChatManager::UChatManager() {
}

UChatManager::~UChatManager() {
}

void UChatManager::Init() {
}

MANAGER_IMPL(UChatManager)

awaitable<void> protocol::C2W_ChatRoomRequest(const std::shared_ptr<IBasePlayer> &plr, IPackage *pkg) {
    // TODO
    co_return;
}

awaitable<void> protocol::C2W_ChatToRoomRequest(const std::shared_ptr<IBasePlayer> &plr, IPackage *pkg) {
    co_return;
}

awaitable<void> protocol::Q2W_ChatResponse(IPackage *pkg) {
    co_return;
}
