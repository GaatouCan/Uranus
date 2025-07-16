#include "Argument.h"
#include "Utils.h"

FArgument::FArgument()
    : mIndex(0) {
}

FArgument::FArgument(const std::string &origin)
    : mOrigin(origin),
      mIndex(0) {
    mArgs = utils::SplitString(mOrigin, ' ');
}

void FArgument::ResetIndex() {
    mIndex = 0;
}

int FArgument::ReadInt() {
    if (mIndex >= mArgs.size()) {
        return 0;
    }

    const std::string str = mArgs[mIndex++];
    return atoi(str.c_str());
}

int FArgument::ReadLong() {
    if (mIndex >= mArgs.size()) {
        return 0;
    }

    const std::string str = mArgs[mIndex++];
    return atoll(str.c_str());
}

std::string FArgument::ReadString() {
    if (mIndex >= mArgs.size()) {
        return {};
    }

    return mArgs[mIndex++];
}
