#include "ClientCommand.h"

IClientCommand::IClientCommand(UCommandObject param)
    : IBaseCommand(std::move(param)), mSender(0) {
}

void IClientCommand::SetSender(const uint32_t sender) {
    mSender = sender;
}

uint32_t IClientCommand::GetSender() const {
    return mSender;
}
