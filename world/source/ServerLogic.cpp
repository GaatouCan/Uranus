#include "../include/ServerLogic.h"

IServerLogic::IServerLogic(UGameWorld *world)
    : mWorld(world) {
}

IServerLogic::~IServerLogic() {
}

UGameWorld *IServerLogic::GetWorld() const {
    return mWorld;
}
