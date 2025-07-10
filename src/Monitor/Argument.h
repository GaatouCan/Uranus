#pragma once

#include "common.h"

#include <vector>
#include <string>

class BASE_API FArgument final {

    std::string mOrigin;
    std::vector<std::string> mArgs;

    int mIndex;

public:
    FArgument();

    explicit FArgument(const std::string &origin);

    void ResetIndex();

    int ReadInt();
    int ReadLong();
    std::string ReadString();
};

