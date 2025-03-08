//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/operate_command.h"
#include "../../../include/system/command/command_system.h"


IOperateCommand::IOperateCommand(UCommandSystem *owner, UCommandObject object)
    : IBaseCommand(owner, std::move(object)),
      operateID_(-1),
      createTime_(0),
      updateTime_(0) {
}

void IOperateCommand::setCommandID(const int64_t id) {
    operateID_ = id;
}

void IOperateCommand::setCreateTime(const int64_t time) {
    createTime_ = time;
}

void IOperateCommand::setUpdateTime(const int64_t time) {
    updateTime_ = time;
}

void IOperateCommand::setCreator(const std::string &creator) {
    creator_ = creator;
}

int64_t IOperateCommand::getCommandID() const {
    return operateID_;
}

int64_t IOperateCommand::getCreateTime() const {
    return createTime_;
}

int64_t IOperateCommand::getUpdateTime() const {
    return updateTime_;
}

std::string IOperateCommand::getCreator() const {
    return creator_;
}
