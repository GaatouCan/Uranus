#pragma once

#include "../../PlayerComponent.h"

class ULottery final : public IPlayerComponent {

public:
    ULottery();
    ~ULottery() override;

    void DoLotto(int num);
};
