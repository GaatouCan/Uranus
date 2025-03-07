//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/operate_command.h"
#include "../../../include/system/command/command_system.h"


IOperateCommand::IOperateCommand(UCommandSystem *owner, UCommandObject object)
    : IBaseCommand(owner, std::move(object)),
      op_id_(-1),
      create_time_(0),
      update_time_(0) {
}

void IOperateCommand::SetCommandID(const int64_t id) {
    op_id_ = id;
}

void IOperateCommand::SetCreateTime(const int64_t time) {
    create_time_ = time;
}

void IOperateCommand::SetUpdateTime(const int64_t time) {
    update_time_ = time;
}

void IOperateCommand::SetCreator(const std::string &creator) {
    creator_ = creator;
}

int64_t IOperateCommand::GetCommandID() const {
    return op_id_;
}

int64_t IOperateCommand::GetCreateTime() const {
    return create_time_;
}

int64_t IOperateCommand::GetUpdateTime() const {
    return update_time_;
}

std::string IOperateCommand::GetCreator() const {
    return creator_;
}
