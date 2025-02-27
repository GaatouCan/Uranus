//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/base_command.h"
#include "../../../include/system/command/command_system.h"

IBaseCommand::IBaseCommand(CommandSystem *owner, CommandObject object)
    : owner_(owner),
      object_(std::move(object)) {
}

CommandSystem * IBaseCommand::GetOwner() const {
    return owner_;
}

GameWorld * IBaseCommand::GetWorld() const {
    return owner_->GetWorld();
}
