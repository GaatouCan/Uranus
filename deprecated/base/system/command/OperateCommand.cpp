#include "OperateCommand.h"

IOperateCommand::IOperateCommand(UCommandObject param)
    : IBaseCommand(std::move(param)),
      mCommandID(0),
      mCreateTime(0),
      mUpdateTime(0) {
}

void IOperateCommand::SetCommandID(const uint64_t id) {
    mCommandID = id;
}

void IOperateCommand::SetCreateTime(const uint64_t time) {
    mCreateTime = time;
}

void IOperateCommand::SetCreator(const std::string &creator) {
    mCreator = creator;
}

uint64_t IOperateCommand::GetCommandID() const {
    return mCommandID;
}

uint64_t IOperateCommand::GetCreateTime() const {
    return mCreateTime;
}

uint64_t IOperateCommand::GetUpdateTime() const {
    return mUpdateTime;
}

std::string IOperateCommand::GetCreator() const {
    return mCreator;
}
