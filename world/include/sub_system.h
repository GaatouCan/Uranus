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

    virtual void init() = 0;

    [[nodiscard]] virtual const char *getSystemName() const = 0;

    [[nodiscard]] UGameWorld *getWorld() const;
    [[nodiscard]] asio::io_context &getIOContext() const;
};

template<class T>
concept CSystemType = std::derived_from<T, ISubSystem>;

#define GET_SYSTEM_NAME(sys) \
[[nodiscard]] constexpr const char *getSystemName() const noexcept override { \
    return #sys; \
}
