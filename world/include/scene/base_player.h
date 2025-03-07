#pragma once

#include "../player_id.h"
#include "../actor.h"
#include "../repeated_timer.h"
#include "../platform_info.h"

#include <map>
#include <shared_mutex>
#include <spdlog/spdlog.h>

class UConnection;
class IPackage;

using AConnectionPointer = std::shared_ptr<UConnection>;


class BASE_API IBasePlayer : public UActor, public std::enable_shared_from_this<IBasePlayer> {

    class IBaseScene *owner_;

    AConnectionPointer conn_;
    FPlayerID pid_;

    ATimePoint enter_time_;
    ATimePoint leave_time_;

    FPlatformInfo platform_;

    std::map<FUniqueID, URepeatedTimer *> timer_map_;
    mutable std::shared_mutex timer_mtx_;

public:
    IBasePlayer() = delete;

    explicit IBasePlayer(const AConnectionPointer &conn);
    ~IBasePlayer() override;

    bool SetConnection(const AConnectionPointer &conn);

    [[nodiscard]] AConnectionPointer GetConnection() const;
    [[nodiscard]] ATcpSocket &GetSocket() const;
    asio::io_context &GetIOContext() const;

    [[nodiscard]] UGameWorld *GetWorld() const;

    [[nodiscard]] AThreadID GetThreadID() const;
    bool IsSameThread() const;

    [[nodiscard]] int32_t GetLocalID() const;
    [[nodiscard]] int32_t GetCrossID() const;
    [[nodiscard]] const FPlayerID &GetPlayerID() const;
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

    ATimePoint GetEnterSceneTime() const;
    ATimePoint GetLeaveSceneTime() const;

    void SetPlatformInfo(const FPlatformInfo &platform);
    const FPlatformInfo &GetPlatformInfo() const;

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
    std::optional<FUniqueID> SetTimer(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Args &&... args)
    {
        const auto timer = new URepeatedTimer(GetIOContext());

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
    std::optional<FUniqueID> SetTimerForMember(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Object *obj, Args &&... args)
    {
        const auto timer = new URepeatedTimer(GetIOContext());
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

    bool StopTimer(const FUniqueID &tid);
    void CleanAllTimer();

    URepeatedTimer *GetTimer(const FUniqueID &tid);

private:
    std::optional<FUniqueID> AddTimer(URepeatedTimer *timer);
    bool RemoveTimer(const FUniqueID &tid);
};
