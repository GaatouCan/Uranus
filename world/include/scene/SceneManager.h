#pragma once

#include <asio.hpp>
#include <atomic>
#include <thread>

#include "../common.h"


static constexpr int32_t kNormalSceneIDBegin = 1000;


class IAbstractScene;

class BASE_API USceneManager final
{
    friend class UGameWorld;

    explicit USceneManager(UGameWorld *world);
    ~USceneManager();

public:
    USceneManager() = delete;

    DISABLE_COPY_MOVE(USceneManager)

    void Init();

    UGameWorld *GetWorld() const;
    IAbstractScene *GetNextMainScene();

    IAbstractScene *GetScene(int32_t sid) const;

private:
    UGameWorld *world_;

    std::vector<IAbstractScene *> mainSceneVec_;
    std::vector<asio::io_context::work> workVec_;
    std::vector<std::thread> threadVec_;

    std::atomic_size_t nextIndex_;

    std::unordered_map<int32_t, IAbstractScene *> sceneMap_;
};
