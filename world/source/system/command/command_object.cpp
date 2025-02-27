//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/command_object.h"
#include "../../../include/utils.h"

#include <spdlog/spdlog.h>


CommandObject::CommandObject(const std::string &type, const std::string &param)
    : type_(type),
      input_(param),
      idx_(0) {
    param_ = utils::SplitString(input_, '|');
}

std::string CommandObject::GetType() const {
    return type_;
}

std::string CommandObject::GetInputString() const {
    return input_;
}

void CommandObject::Reset() {
    idx_ = 0;
}

int CommandObject::ReadInt() {
    if (idx_ >= param_.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return -1;
    }
    return std::stoi(param_[idx_++]);
}

unsigned int CommandObject::ReadUInt() {
    if (idx_ >= param_.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return -1;
    }
    return std::stoul(param_[idx_++]);
}

std::string CommandObject::ReadString() {
    if (idx_ >= param_.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return {};
    }
    return param_[idx_++];
}
