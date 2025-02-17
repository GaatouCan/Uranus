#pragma once

#include <asio.hpp>
#include <atomic>
#include <thread>

#include "../common.h"


static constexpr int32_t NORMAL_SCENE_ID_BEGIN = 1000;


class IBaseScene;
class GameWorld;

class BASE_API SceneManager final
{
    friend class GameWorld;

    explicit SceneManager(GameWorld *world);
    ~SceneManager();

public:
    SceneManager() = delete;

    DISABLE_COPY_MOVE(SceneManager)

    void Init();

    GameWorld *GetWorld() const;
    IBaseScene *GetNextMainScene();

    IBaseScene *GetScene(int32_t sid) const;

private:
    GameWorld *mWorld;

    std::vector<IBaseScene *> mMainSceneList;
    std::vector<asio::io_context::work> mWorkList;
    std::vector<std::thread> mThreadList;

    std::atomic_size_t mNextMainIndex;

    std::unordered_map<int32_t, IBaseScene *> mSceneMap;
};
