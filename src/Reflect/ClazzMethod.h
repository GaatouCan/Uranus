#pragma once

#include "../common.h"

#include <string>

class BASE_API FClazzMethod final {

public:
    FClazzMethod() = delete;

    FClazzMethod(std::string name, uintptr_t ptr);
    ~FClazzMethod();

    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] uintptr_t GetPointer() const;

private:
    const std::string mName;
    uintptr_t mMethodPtr;
};

