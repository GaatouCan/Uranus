#pragma once

#include <string>
#include <vector>

#ifdef __linux__
#include <cstdint>
#endif

#include "../../common.h"

class CommandObject final {
    std::string mType;
    std::string mInput;
    std::vector<std::string> mParam;
    size_t mIndex;

public:
    CommandObject() = delete;

    explicit CommandObject(const std::string &type, const std::string &param = "");

    [[nodiscard]] std::string GetType() const;
    [[nodiscard]] std::string GetInputString() const;

    void Reset();

    int ReadInt();
    unsigned int ReadUInt();
    std::string ReadString();
};