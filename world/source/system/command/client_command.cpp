//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/client_command.h"
#include "../../../include/system/command/command_system.h"


IClientCommand::IClientCommand(CommandSystem *owner, CommandObject object)
    : IBaseCommand(owner, std::move(object)),
      sender_(-1) {
}

void IClientCommand::SetSender(const int32_t sender) {
    sender_ = sender;
}

int32_t IClientCommand::GetSender() const {
    return sender_;
}
