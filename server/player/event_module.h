#pragma once

#include "../common/event.h"

#include "system/event/event_system.h"

#include <queue>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <spdlog/spdlog.h>
#include <asio.hpp>


class EventModule final {

    class Player *owner_;

    struct EventNode {
        EEvent event = EEvent::UNAVAILABLE;
        IEventParam *param = nullptr;
    };

    std::queue<EventNode> queue_;
    mutable std::shared_mutex event_mtx_;

    std::map<EEvent, std::map<void *, AEventListener>> listener_map_;
    std::map<void *, AEventListener> cur_listener_;
    std::mutex listener_mtx_;

public:
    EventModule() = delete;

    explicit EventModule(Player *plr);
    ~EventModule();

    [[nodiscard]] Player *GetOwner() const;

    [[nodiscard]] bool IsQueueEmpty() const;

    template<typename Target, typename Callable>
    void RegisterListenerT(const EEvent event, void *ptr, void *target, Callable && func) {
        if (event == EEvent::UNAVAILABLE || ptr == nullptr || target == nullptr)
            return;

        this->RegisterListener(event, ptr, [target, func = std::forward<Callable>(func)](IEventParam *param) {
            try {
                if (target != nullptr) {
                    std::invoke(func, static_cast<Target *>(target), param);
                }
            } catch (std::exception &e) {
                spdlog::warn("Player Event Module - {}", e.what());
            }
        });
    }

    void RegisterListener(EEvent event, void *ptr, const AEventListener &listener);
    void RemoveListener(EEvent event, void *ptr);

    void Dispatch(EEvent event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);

private:
    asio::awaitable<void> HandleEvent();
};
