#include "../include/SubSystem.h"

#include "../include/GameWorld.h"

ISubSystem::ISubSystem(UGameWorld *world)
    : world_(world) {

}

UGameWorld *ISubSystem::GetWorld() const {
    return world_;
}

asio::io_context & ISubSystem::GetIOContext() const {
    return world_->GetIOContext();
}

