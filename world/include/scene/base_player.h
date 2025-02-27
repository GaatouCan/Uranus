#pragma once

#include "../player_id.h"
#include "../actor.h"
#include "../repeated_timer.h"
#include "../platform_info.h"

#include <map>
#include <shared_mutex>
#include <spdlog/spdlog.h>

class Connection;
class IPackage;

using ConnectionPointer = std::shared_ptr<Connection>;


class BASE_API IBasePlayer : public Actor, public std::enable_shared_from_this<IBasePlayer> {

    class IBaseScene *owner_;

    ConnectionPointer conn_;
    PlayerID pid_;

    TimePoint enter_time_;
    TimePoint leave_time_;

    PlatformInfo platform_;

    std::map<UniqueID, RepeatedTimer *> timer_map_;
    mutable std::shared_mutex timer_mtx_;

public:
    IBasePlayer() = delete;

    explicit IBasePlayer(ConnectionPointer conn);
    ~IBasePlayer() override;

    bool SetConnection(ConnectionPointer conn);

    [[nodiscard]] ConnectionPointer GetConnection() const;
    [[nodiscard]] TcpSocket &GetSocket() const;
    asio::io_context &GetIOContext() const;

    [[nodiscard]] GameWorld *GetWorld() const;

    [[nodiscard]] ThreadID GetThreadID() const;
    bool IsSameThread() const;

    [[nodiscard]] int32_t GetLocalID() const;
    [[nodiscard]] int32_t GetCrossID() const;
    [[nodiscard]] const PlayerID &GetPlayerID() const;
    [[nodiscard]] int64_t GetFullID() const;

    [[nodiscard]] IPackage *BuildPackage() const;
    void SendPackage(IPackage *pkg) const;

    // 如果继承 请将基类调用放在最顶部
    virtual void OnEnterScene(IBaseScene *scene);

    // 如果继承 请将基类调用放在最底部
    virtual void OnLeaveScene(IBaseScene *scene);

    bool TryLeaveScene();

    bool IsInScene(int32_t id = 0) const;
    int32_t GetCurrentSceneID() const;

    [[nodiscard]] IBaseScene *GetCurrentScene() const;

    TimePoint GetEnterSceneTime() const;
    TimePoint GetLeaveSceneTime() const;

    void SetPlatformInfo(const PlatformInfo &platform);
    const PlatformInfo &GetPlatformInfo() const;

    template<typename Functor, typename... Args>
    void RunInThread(Functor &&func, Args &&... args) {
        co_spawn(GetIOContext(), [func = std::forward<Functor>(func), ...args = std::forward<Args>(args)]() mutable -> awaitable<void> {
            try {
                std::invoke(func, args...);
            } catch (std::exception &e) {
                spdlog::error("IBasePlayer::runInThread: {}", e.what());
            }
            co_return;
        }, detached);
    }

    template<typename Functor, typename... Args>
    std::optional<UniqueID> SetTimer(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Args &&... args)
    {
        const auto timer = new RepeatedTimer(GetIOContext());

        timer->SetDelay(delay)
            .SetRepeatRate(rate)
            .SetIfRepeat(repeat)
            .SetFunctor(std::forward<Functor>(func), std::forward<Args>(args)...);

        const auto tid = AddTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    template<typename Functor, typename Object, typename... Args>
    std::optional<UniqueID> SetTimerForMember(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Object *obj, Args &&... args)
    {
        const auto timer = new RepeatedTimer(GetIOContext());
        timer->SetDelay(delay)
            .SetRepeatRate(rate)
            .SetIfRepeat(repeat)
            .SetMemberFunctor(std::forward<Functor>(func), obj, std::forward<Args>(args)...);

        const auto tid = AddTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    bool StopTimer(const UniqueID &tid);
    void CleanAllTimer();

    RepeatedTimer *GetTimer(const UniqueID &tid);

private:
    std::optional<UniqueID> AddTimer(RepeatedTimer *timer);
    bool RemoveTimer(const UniqueID &tid);
};
