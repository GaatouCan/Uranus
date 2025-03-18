#pragma once

#include "scene/special_scene.h"

struct FCoffinNode {
    uint32_t id;
};

struct FMonsterNode {
    uint32_t id;
};

class UFloatingCoffin final : public ISpecialScene {

public:
    UFloatingCoffin(USceneManager *owner, int32_t id);
    ~UFloatingCoffin() override;
};

