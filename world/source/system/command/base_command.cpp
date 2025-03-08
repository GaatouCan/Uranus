//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/base_command.h"
#include "../../../include/system/command/command_system.h"

IBaseCommand::IBaseCommand(UCommandSystem *owner, UCommandObject object)
    : owner_(owner),
      object_(std::move(object)) {
}

UCommandSystem * IBaseCommand::getOwner() const {
    return owner_;
}

UGameWorld * IBaseCommand::getWorld() const {
    return owner_->getWorld();
}
