#pragma once

#include "../utils.h"

#include <map>
#include <set>
#include <shared_mutex>
#include <spdlog/spdlog.h>


class IBasePlayer;

class BASE_API IBaseScene {

    class SceneManager* owner_;
    const int32_t scene_id_;

    std::map<int32_t, std::shared_ptr<IBasePlayer>> player_map_;
    mutable std::shared_mutex mtx_;

public:
    IBaseScene() = delete;

    IBaseScene(SceneManager *owner, int32_t id);
    virtual ~IBaseScene();

    [[nodiscard]] int32_t GetSceneID() const;
    [[nodiscard]] SceneManager* GetOwner() const;
    [[nodiscard]] GameWorld *GetWorld() const;

    void PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player);
    void PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, bool bChange = false);

    std::shared_ptr<IBasePlayer> GetPlayer(int32_t pid) const;

    void RunInThread(const std::function<awaitable<void>()> &func) const;
    void RunInThread(std::function<awaitable<void>()> &&func) const;

    template<typename Functor, typename... Args>
    void PushTask(Functor &&func, Args &&... args)
    {
        this->RunInThread([func = std::forward<Functor>(func), ...args = std::forward<Args>(args)]() -> awaitable<void> {
            try {
                std::invoke(func, args...);
            } catch (std::exception &e) {
                spdlog::error("IAbstractScene::PushTask: {}", e.what());
            }
            co_return;
        });
    }

    std::set<std::shared_ptr<IBasePlayer>> GetPlayerInScene() const;

    // Default true
    virtual bool CanDestroy() const;

public:
    TimePoint destroy_time_point_;
};
