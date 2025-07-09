#pragma once

#include "../../PlayerComponent.h"
#include "ConsumeType.h"

struct FItemData {
    int item_id;
    int num;
};


class UKnapsack final : public IPlayerComponent {

public:
    UKnapsack();

    bool ConsumeItem(const std::vector<FItemData> &list, EConsumeType type = EConsumeType::NORMAL, const char *reason = "");
};

