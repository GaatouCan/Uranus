#pragma once

#include "../common/event.h"

#include "system/event/event_system.h"

#include <queue>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <spdlog/spdlog.h>
#include <asio.hpp>


class UEventModule final {

    class UPlayer *owner_;

    struct FEventNode {
        EEvent event = EEvent::UNAVAILABLE;
        IEventParam *param = nullptr;
    };

    std::queue<FEventNode> queue_;
    mutable std::shared_mutex eventMutex_;

    std::map<EEvent, std::map<void *, AEventListener>> listenerMap_;
    std::map<void *, AEventListener> curListener_;
    std::mutex listenerMutex_;

public:
    UEventModule() = delete;

    explicit UEventModule(UPlayer *plr);
    ~UEventModule();

    [[nodiscard]] UPlayer *getOwner() const;

    [[nodiscard]] bool queueEmpty() const;

    template<typename Target, typename Callable>
    void registerListenerT(const EEvent event, void *ptr, void *target, Callable && func) {
        if (event == EEvent::UNAVAILABLE || ptr == nullptr || target == nullptr)
            return;

        registerListener(event, ptr, [target, func = std::forward<Callable>(func)](IEventParam *param) {
            try {
                if (target != nullptr) {
                    std::invoke(func, static_cast<Target *>(target), param);
                }
            } catch (std::exception &e) {
                spdlog::warn("Player Event Module - {}", e.what());
            }
        });
    }

    void registerListener(EEvent event, void *ptr, const AEventListener &listener);
    void removeListener(EEvent event, void *ptr);

    void dispatch(EEvent event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);

private:
    asio::awaitable<void> handleEvent();
};
