#include "../include/sub_system.h"
#include "../include/game_world.h"

ISubSystem::ISubSystem(UGameWorld *world)
    : world_(world) {

}

UGameWorld *ISubSystem::getWorld() const {
    return world_;
}

asio::io_context & ISubSystem::getIOContext() const {
    return world_->getIOContext();
}

