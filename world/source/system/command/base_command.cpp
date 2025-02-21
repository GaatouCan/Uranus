//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/base_command.h"
#include "../../../include/system/command/command_system.h"

IBaseCommand::IBaseCommand(CommandSystem *owner, CommandObject object)
    : mOwner(owner),
      mObject(std::move(object)) {
}

CommandSystem * IBaseCommand::GetOwner() const {
    return mOwner;
}

GameWorld * IBaseCommand::GetWorld() const {
    return mOwner->GetWorld();
}
