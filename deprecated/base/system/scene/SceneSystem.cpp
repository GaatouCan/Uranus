#include "SceneSystem.h"
#include "Scene.h"
#include "../../GameWorld.h"

constexpr int kSceneCount = 10;

USceneSystem::USceneSystem() {

}

USceneSystem::~USceneSystem() {
    for (const auto scene : mSceneVector) {
        delete scene;
    }
}

void USceneSystem::Init() {
    const auto& cfg = UGameWorld::Instance().GetServerConfig();

    size_t index = 0;
    for (const auto it : cfg["scene"]["main"]) {
        auto scene = new UScene(index++);
        mSceneVector.push_back(scene);

        spdlog::info("\tCreated Main Scene[{}].", scene->GetSceneID());
    }

    for (const auto it : cfg["scene"]["state"]) {
        auto scene = new UScene(index++);
        mSceneVector.push_back(scene);

        spdlog::info("\tCreated State Scene[{}].", scene->GetSceneID());
    }
}

SUB_SYSTEM_IMPL(USceneSystem)

UScene * USceneSystem::GetMainScene() const {
    return GetSceneByID(1);
}

UScene * USceneSystem::GetSceneByID(const uint32_t id) const {
    if (id >= mSceneVector.size()) {
        return nullptr;
    }
    return mSceneVector[id];
}
