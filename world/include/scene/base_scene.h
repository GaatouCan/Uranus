#pragma once

#include "../utils.h"

#include <map>
#include <set>
#include <shared_mutex>
#include <spdlog/spdlog.h>


class IBasePlayer;

class BASE_API IBaseScene {

    class USceneManager* owner_;
    const int32_t sceneID_;

    std::map<int32_t, std::shared_ptr<IBasePlayer>> playerMap_;
    mutable std::shared_mutex mutex_;

public:
    IBaseScene() = delete;

    IBaseScene(USceneManager *owner, int32_t id);
    virtual ~IBaseScene();

    [[nodiscard]] int32_t getSceneID() const;
    [[nodiscard]] USceneManager* getOwner() const;
    [[nodiscard]] UGameWorld *getWorld() const;

    void playerEnterScene(const std::shared_ptr<IBasePlayer> &player);
    void playerLeaveScene(const std::shared_ptr<IBasePlayer> &player, bool bChange = false);

    std::shared_ptr<IBasePlayer> findPlayer(int32_t pid) const;

    void runInThread(const std::function<awaitable<void>()> &func) const;
    void runInThread(std::function<awaitable<void>()> &&func) const;

    template<typename Functor, typename... Args>
    void pushTask(Functor &&func, Args &&... args)
    {
        runInThread([func = std::forward<Functor>(func), ...args = std::forward<Args>(args)]() -> awaitable<void> {
            try {
                std::invoke(func, args...);
            } catch (std::exception &e) {
                spdlog::error("IAbstractScene::PushTask: {}", e.what());
            }
            co_return;
        });
    }

    std::set<std::shared_ptr<IBasePlayer>> getPlayerInScene() const;

    // Default true
    virtual bool canDestroy() const;

public:
    ATimePoint destroyTimePoint_;
};
