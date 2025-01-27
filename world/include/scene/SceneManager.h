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
    UGameWorld *mWorld;

    std::vector<IAbstractScene *> mMainVec;
    std::vector<asio::io_context::work> mWorkVec;
    std::vector<std::thread> mThreadVec;

    std::atomic_size_t mNextIndex;

    std::unordered_map<int32_t, IAbstractScene *> mSceneMap;
};
