#include "../include/sub_system.h"
#include "../include/game_world.h"

ISubSystem::ISubSystem(GameWorld *world)
    : mWorld(world) {

}

GameWorld *ISubSystem::GetWorld() const {
    return mWorld;
}

asio::io_context & ISubSystem::GetIOContext() const {
    return mWorld->GetIOContext();
}

