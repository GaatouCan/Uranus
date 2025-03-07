//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/base_command.h"
#include "../../../include/system/command/command_system.h"

IBaseCommand::IBaseCommand(UCommandSystem *owner, UCommandObject object)
    : owner_(owner),
      object_(std::move(object)) {
}

UCommandSystem * IBaseCommand::GetOwner() const {
    return owner_;
}

UGameWorld * IBaseCommand::GetWorld() const {
    return owner_->GetWorld();
}
