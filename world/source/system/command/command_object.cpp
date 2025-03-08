//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/command_object.h"
#include "../../../include/utils.h"

#include <spdlog/spdlog.h>


UCommandObject::UCommandObject(const std::string &type, const std::string &param)
    : type_(type),
      input_(param),
      idx_(0) {
    param_ = utils::SplitString(input_, '|');
}

std::string UCommandObject::getType() const {
    return type_;
}

std::string UCommandObject::getInputString() const {
    return input_;
}

void UCommandObject::reset() {
    idx_ = 0;
}

int UCommandObject::readInt() {
    if (idx_ >= param_.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return -1;
    }
    return std::stoi(param_[idx_++]);
}

unsigned int UCommandObject::readUInt() {
    if (idx_ >= param_.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return -1;
    }
    return std::stoul(param_[idx_++]);
}

std::string UCommandObject::readString() {
    if (idx_ >= param_.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return {};
    }
    return param_[idx_++];
}
