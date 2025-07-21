#pragma once

#include "Common.h"

#include <concepts>

/**
 * Abstract Base Class Of Internal Data Exchange With The Server
 */
class BASE_API IPackageInterface {

public:
    IPackageInterface() = default;
    virtual ~IPackageInterface() = default;

    virtual void SetPackageID(uint32_t id) = 0;
    virtual void SetSource(int32_t source) = 0;
    virtual void SetTarget(int32_t target) = 0;

    [[nodiscard]] virtual uint32_t GetID() const = 0;
    [[nodiscard]] virtual int32_t GetSource() const = 0;
    [[nodiscard]] virtual int32_t GetTarget() const = 0;
};

template<typename T>
concept CPackageType = std::derived_from<T, IPackageInterface> && !std::is_same_v<T, IPackageInterface>;
