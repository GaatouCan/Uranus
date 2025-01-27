#include "../include/SubSystem.h"

#include "../include/GameWorld.h"

ISubSystem::ISubSystem(UGameWorld *world)
    : mWorld(world) {

}

UGameWorld *ISubSystem::GetWorld() const {
    return mWorld;
}

asio::io_context & ISubSystem::GetIOContext() const {
    return mWorld->GetIOContext();
}

