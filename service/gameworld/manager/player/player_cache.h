#pragma once


#include <string>

struct FPlayerCache final {
    int64_t pid;
    std::string name;

    long long loginTime;
    long long logoutTime;
    long long syncTime;

    int avatarIndex;

    [[nodiscard]] bool IsOnline() const;
};
