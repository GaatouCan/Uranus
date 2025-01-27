#pragma once

#include <string>
#include <vector>

#ifdef __linux__
#include <cstdint>
#endif

class UCommandObject final {

    std::string mCmdType;
    std::string mCmdOriginParam;
    std::vector<std::string> mCmdParams;
    size_t mIndex;

public:
    UCommandObject() = delete;
    explicit UCommandObject(std::string cmdType, std::string cmdOriginParam = "");

    [[nodiscard]] std::string GetType() const;
    [[nodiscard]] std::string GetParamString() const;

    void Reset();

    int ReadInt();
    unsigned int ReadUInt();
    std::string ReadString();
};


