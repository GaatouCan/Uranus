#include "CommandObject.h"
#include "../../utils.h"

#include <utility>
#include <spdlog/spdlog.h>


UCommandObject::UCommandObject(std::string cmdType, std::string cmdOriginParam)
    : mCmdType(std::move(cmdType)),
      mCmdOriginParam(std::move(cmdOriginParam)),
      mCmdParams(utils::SplitString(mCmdOriginParam, '|')),
      mIndex(0) {
}

std::string UCommandObject::GetType() const {
    return mCmdType;
}

std::string UCommandObject::GetParamString() const {
    return mCmdOriginParam;
}

void UCommandObject::Reset() {
    mIndex = 0;
}

int UCommandObject::ReadInt() {
    if (mIndex >= mCmdParams.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return -1;
    }
    return std::stoi(mCmdParams[mIndex++]);
}

unsigned int UCommandObject::ReadUInt() {
    if (mIndex >= mCmdParams.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return 0;
    }
    return std::stoul(mCmdParams[mIndex++]);
}

std::string UCommandObject::ReadString() {
    if (mIndex >= mCmdParams.size()) {
        spdlog::error("{} - index out of range", __FUNCTION__);
        return {};
    }
    return mCmdParams[mIndex++];
}
