#pragma once

#include "Common.h"

class BASE_API IDataAssetInterface {

public:
    virtual ~IDataAssetInterface() = default;

    void LoadSynchronous();
};
