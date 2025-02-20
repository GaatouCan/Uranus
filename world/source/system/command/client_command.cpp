//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/client_command.h"

IClientCommand::IClientCommand(CommandObject object)
    : IBaseCommand(std::move(object)),
      mSender(-1) {
}

void IClientCommand::SetSender(const int32_t sender) {
    mSender = sender;
}

int32_t IClientCommand::GetSender() const {
    return mSender;
}
