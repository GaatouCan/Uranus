#pragma once

#include <string>
#include <vector>

#ifdef __linux__
#include <cstdint>
#endif

#include "../../common.h"

class BASE_API UCommandObject final {
    std::string type_;
    std::string input_;
    std::vector<std::string> param_;
    size_t idx_;

public:
    UCommandObject() = delete;

    explicit UCommandObject(const std::string &type, const std::string &param = "");

    [[nodiscard]] std::string GetType() const;
    [[nodiscard]] std::string GetInputString() const;

    void Reset();

    int ReadInt();
    unsigned int ReadUInt();
    std::string ReadString();
};