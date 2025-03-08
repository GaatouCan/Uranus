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

    ATimePoint enterTime_;
    ATimePoint leaveTime_;

    FPlatformInfo platform_;

    std::map<FUniqueID, URepeatedTimer *> timerMap_;
    mutable std::shared_mutex timerMutex_;

public:
    IBasePlayer() = delete;

    explicit IBasePlayer(const AConnectionPointer &conn);
    ~IBasePlayer() override;

    bool setConnection(const AConnectionPointer &conn);

    [[nodiscard]] AConnectionPointer getConnection() const;
    [[nodiscard]] ATcpSocket &getSocket() const;
    asio::io_context &getIOContext() const;

    [[nodiscard]] UGameWorld *getWorld() const;

    [[nodiscard]] AThreadID getThreadID() const;
    bool isSameThread() const;

    [[nodiscard]] int32_t getLocalID() const;
    [[nodiscard]] int32_t getCrossID() const;
    [[nodiscard]] const FPlayerID &getPlayerID() const;
    [[nodiscard]] int64_t getFullID() const;

    [[nodiscard]] IPackage *buildPackage() const;
    void sendPackage(IPackage *pkg) const;

    // 如果继承 请将基类调用放在最顶部
    virtual void onEnterScene(IBaseScene *scene);

    // 如果继承 请将基类调用放在最底部
    virtual void onLeaveScene(IBaseScene *scene);

    bool tryLeaveScene();

    bool inScene(int32_t id = 0) const;
    int32_t getCurrentSceneID() const;

    [[nodiscard]] IBaseScene *getCurrentScene() const;

    ATimePoint getEnterSceneTime() const;
    ATimePoint getLeaveSceneTime() const;

    void setPlatformInfo(const FPlatformInfo &platform);
    const FPlatformInfo &getPlatformInfo() const;

    template<typename Functor, typename... Args>
    void runInThread(Functor &&func, Args &&... args) {
        co_spawn(getIOContext(), [func = std::forward<Functor>(func), ...args = std::forward<Args>(args)]() mutable -> awaitable<void> {
            try {
                std::invoke(func, args...);
            } catch (std::exception &e) {
                spdlog::error("IBasePlayer::runInThread: {}", e.what());
            }
            co_return;
        }, detached);
    }

    template<typename Functor, typename... Args>
    std::optional<FUniqueID> setTimer(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Args &&... args)
    {
        const auto timer = new URepeatedTimer(getIOContext());

        timer->SetDelay(delay)
            .SetRepeatRate(rate)
            .SetIfRepeat(repeat)
            .SetFunctor(std::forward<Functor>(func), std::forward<Args>(args)...);

        const auto tid = addTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    template<typename Functor, typename Object, typename... Args>
    std::optional<FUniqueID> setTimerForMember(
        const std::chrono::duration<uint32_t> delay,
        const std::chrono::duration<uint32_t> rate,
        const bool repeat,
        Functor &&func, Object *obj, Args &&... args)
    {
        const auto timer = new URepeatedTimer(getIOContext());
        timer->SetDelay(delay)
            .SetRepeatRate(rate)
            .SetIfRepeat(repeat)
            .SetMemberFunctor(std::forward<Functor>(func), obj, std::forward<Args>(args)...);

        const auto tid = addTimer(timer);
        if (!tid.has_value()) {
            delete timer;
        }
        return tid;
    }

    bool stopTimer(const FUniqueID &tid);
    void cleanAllTimer();

    URepeatedTimer *getTimer(const FUniqueID &tid);

private:
    std::optional<FUniqueID> addTimer(URepeatedTimer *timer);
    bool removeTimer(const FUniqueID &tid);
};
