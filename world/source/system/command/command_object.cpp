//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/command_object.h"
#include "../../../include/utils.h"

#include <spdlog/spdlog.h>


CommandObject::CommandObject(const std::string &type, const std::string &param)
    : mType(type),
      mInput(param),
      mIndex(0) {
    mParam = utils::SplitString(mInput, '|');
}

std::string CommandObject::GetType() const {
    return mType;
}

std::string CommandObject::GetInputString() const {
    return mInput;
}

void CommandObject::Reset() {
    mIndex = 0;
}

int CommandObject::ReadInt() {
    if (mIndex >= mParam.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return -1;
    }
    return std::stoi(mParam[mIndex++]);
}

unsigned int CommandObject::ReadUInt() {
    if (mIndex >= mParam.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return -1;
    }
    return std::stoul(mParam[mIndex++]);
}

std::string CommandObject::ReadString() {
    if (mIndex >= mParam.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return {};
    }
    return mParam[mIndex++];
}
