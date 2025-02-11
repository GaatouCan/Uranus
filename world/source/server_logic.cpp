#include "../include/ServerLogic.h"

IServerLogic::IServerLogic(GameWorld *world)
    : world_(world) {
}

IServerLogic::~IServerLogic() {
}

GameWorld *IServerLogic::GetWorld() const {
    return world_;
}
