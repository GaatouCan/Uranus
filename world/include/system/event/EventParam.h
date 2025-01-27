#pragma once

#include "../../common.h"
#include <functional>


struct BASE_API IEventParam {
    virtual ~IEventParam() = default;
};


using AEventListener = std::function<void(IEventParam *)>;
