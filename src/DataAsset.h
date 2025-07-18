#pragma once

#include "Common.h"

class BASE_API IDataAsset {

public:
    virtual ~IDataAsset() = default;

    void LoadSynchronous();
};
