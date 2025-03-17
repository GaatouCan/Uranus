#pragma once

#include "recyclable.h"

#include <concepts>

class BASE_API IPackage : public IRecyclable {

public:
    explicit IPackage(IRecycler* handle) : IRecyclable(handle) {}

    [[nodiscard]] virtual uint32_t getPackageID() const { return -1; }
};

template<typename T>
concept CPackageType = std::derived_from<T, IPackage>;
