#pragma once

#include "Recyclable.h"

#include <concepts>

/**
 * Abstract Base Class Of Internal Data Exchange With The Server
 */
class BASE_API IPackageBase : public IRecyclable {

public:
    IPackageBase() = default;

    virtual void SetID(uint32_t id) = 0;
    virtual void SetSource(int32_t source) = 0;
    virtual void SetTarget(int32_t target) = 0;

    [[nodiscard]] virtual uint32_t GetID() const = 0;
    [[nodiscard]] virtual int32_t GetSource() const = 0;
    [[nodiscard]] virtual int32_t GetTarget() const = 0;
};

template<typename T>
concept CPackageType = std::derived_from<T, IPackageBase> && !std::is_same_v<T, IPackageBase>;
