#pragma once

#include "../../SubSystem.h"

class UScene;

class USceneSystem final : public ISubSystem {

    std::vector<UScene *> mSceneVector;

public:
    USceneSystem();
    ~USceneSystem() override;

    SUB_SYSTEM_BODY(USceneSystem)

    void Init() override;

    [[nodiscard]] UScene *GetMainScene() const;
    [[nodiscard]] UScene *GetSceneByID(uint32_t id) const;
};

