#pragma once

#include "../../GameWorld.h"

#include <map>
#include <set>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <spdlog/spdlog.h>

#ifdef __linux__
#include <cstdint>
#endif

class IBasePlayer;
class UConnection;

class UScene final {

    uint32_t mSceneID;

    std::map<uint32_t, std::shared_ptr<IBasePlayer>> mPlayerMap;
    std::mutex mMutex;
    mutable std::shared_mutex mSharedMutex;

public:
    UScene() = delete;

    explicit UScene(uint32_t id);
    ~UScene();

    void SetSceneID(uint32_t id);
    [[nodiscard]] uint32_t GetSceneID() const;

    template<typename FUNC, typename... ARGS>
    void RunInThread(FUNC &&func, ARGS &&... args) {
        co_spawn(UGameWorld::Instance().GetIOContext(), [func = std::forward<FUNC>(func), ...args = std::forward<ARGS>(args)]() -> awaitable<void> {
            try {
                std::invoke(func, args...);
            } catch (std::exception &e) {
                spdlog::error("UScene::runInThread: {}", e.what());
            }
            co_return;
        }, detached);
    }

    void PlayerEnterScene(const std::shared_ptr<IBasePlayer> &player);
    void PlayerLeaveScene(const std::shared_ptr<IBasePlayer> &player, bool bChange = false);

    std::shared_ptr<IBasePlayer> GetPlayer(uint32_t pid) const;

    void BroadCast(class IPackage *pkg, const std::set<uint32_t> &except = {});
};

