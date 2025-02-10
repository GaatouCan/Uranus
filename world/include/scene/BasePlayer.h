#pragma once

#include "../PlayerID.h"
#include "../Actor.h"
#include "../Connection.h"
#include "../RepeatedTimer.h"
#include "../PlatformInfo.h"

#include <map>
#include <shared_mutex>
#include <spdlog/spdlog.h>


class BASE_API IBasePlayer : public UActor, public std::enable_shared_from_this<IBasePlayer> {

    class IAbstractScene *owner_;

    AConnectionPointer conn_;
    FPlayerID pid_;

    ATimePoint enterTime_;
    ATimePoint leaveTime_;

    FPlatformInfo platform_;

    std::map<FUniqueID, URepeatedTimer *> timerMap_;
    mutable std::shared_mutex timerMutex_;

public:
    IBasePlayer() = delete;

    explicit IBasePlayer(AConnectionPointer conn);
    ~IBasePlayer() override;

    bool SetConnection(AConnectionPointer conn);

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
    virtual void OnEnterScene(IAbstractScene *scene);

    // 如果继承 请将基类调用放在最底部
    virtual void OnLeaveScene(IAbstractScene *scene);

    bool TryLeaveScene();

    bool IsInScene(int32_t id = 0) const;
    int32_t GetCurrentSceneID() const;

    [[nodiscard]] IAbstractScene *GetCurrentScene() const;

    ATimePoint GetEnterSceneTime() const;
    ATimePoint GetLeaveSceneTime() const;

    void SetPlatformInfo(const FPlatformInfo &platform);
    const FPlatformInfo &GetPlatformInfo() const;

    template<typename FUNC, typename... ARGS>
    void RunInThread(FUNC &&func, ARGS &&... args) {
        co_spawn(conn_->GetSocket().get_executor(), [func = std::forward<FUNC>(func), ...args = std::forward<ARGS>(args)]() mutable -> awaitable<void> {
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
