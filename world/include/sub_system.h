#pragma once

#include "common.h"

#include <concepts>
#include <asio.hpp>


class BASE_API ISubSystem {

    class UGameWorld *world_;

public:
    ISubSystem() = delete;

    explicit ISubSystem(UGameWorld *world);
    virtual ~ISubSystem() = default;

    DISABLE_COPY_MOVE(ISubSystem)

    virtual void Init() = 0;

    [[nodiscard]] virtual const char *GetSystemName() const = 0;

    [[nodiscard]] UGameWorld *GetWorld() const;
    [[nodiscard]] asio::io_context &GetIOContext() const;
};

template<class T>
concept SYSTEM_TYPE = std::derived_from<T, ISubSystem>;

#define GET_SYSTEM_NAME(sys) \
[[nodiscard]] constexpr const char *GetSystemName() const noexcept override { \
    return #sys; \
}
