#pragma once

#include "../../PlayerComponent.h"

class ULottery final : public IPlayerComponent {

public:
    ULottery();

    void DoLotto(int num);
};
