//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/operate_command.h"

IOperateCommand::IOperateCommand(CommandObject object)
    : IBaseCommand(std::move(object)),
      mOperateID(-1),
      mCreateTime(0),
      mUpdateTime(0) {
}

void IOperateCommand::SetCommandID(const int64_t id) {
    mOperateID = id;
}

void IOperateCommand::SetCreateTime(const int64_t time) {
    mCreateTime = time;
}

void IOperateCommand::SetUpdateTime(const int64_t time) {
    mUpdateTime = time;
}

void IOperateCommand::SetCreator(const std::string &creator) {
    mCreator = creator;
}

int64_t IOperateCommand::GetCommandID() const {
    return mOperateID;
}

int64_t IOperateCommand::GetCreateTime() const {
    return mCreateTime;
}

int64_t IOperateCommand::GetUpdateTime() const {
    return mUpdateTime;
}

std::string IOperateCommand::GetCreator() const {
    return mCreator;
}
