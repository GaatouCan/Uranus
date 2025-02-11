#pragma once

#include "../../common.h"
#include <functional>


struct BASE_API IEventParam {
    virtual ~IEventParam() = default;
};


using EventListener = std::function<void(IEventParam *)>;
