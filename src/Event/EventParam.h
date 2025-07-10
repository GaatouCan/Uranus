#pragma once

#include "common.h"

#include <concepts>


class BASE_API IEventParam {
public:
    IEventParam() = default;
    virtual ~IEventParam() = default;

    virtual int GetEventType() const = 0;
};

template <typename T>
concept CEventType = std::derived_from<T, IEventParam>;
