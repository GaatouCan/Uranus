#include "../include/server_logic.h"

IServerLogic::IServerLogic(GameWorld *world)
    : mWorld(world) {
}

IServerLogic::~IServerLogic() {
}

GameWorld *IServerLogic::GetWorld() const {
    return mWorld;
}
