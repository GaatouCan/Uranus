#pragma once

#include "../utils.h"

#include <map>
#include <shared_mutex>
#include <spdlog/spdlog.h>

class IBasePlayer;

class BASE_API IAbstractScene {

    class USceneManager* owner_;
    const int32_t id_;

    std::map<int32_t, std::shared_ptr<IBasePlayer>> playerMap_;
    mutable std::shared_mutex mutex_;

public:
    IAbstractScene() = delete;

    IAbstractScene(USceneManager *owner, int32_t id);
    virtual ~IAbstractScene();

    [[nodiscard]] int32_t GetSceneID() const;
    [[nodiscard]] USceneManager* GetOwner() const;
    [[nodiscard]] class UGameWorld *GetWorld() const;

    void PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player);
    void PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, bool bChange = false);

    std::shared_ptr<IBasePlayer> GetPlayer(int32_t pid) const;

    void RunInThread(const std::function<awaitable<void>()> &func) const;
    void RunInThread(std::function<awaitable<void>()> &&func) const;

    template<typename FUNC, typename... ARGS>
    void PushTask(FUNC &&func, ARGS &&... args)
    {
        this->RunInThread([func = std::forward<FUNC>(func), ...args = std::forward<ARGS>(args)]() -> awaitable<void> {
            try {
                std::invoke(func, args...);
            } catch (std::exception &e) {
                spdlog::error("IAbstractScene::PushTask: {}", e.what());
            }
            co_return;
        });
    }
};
