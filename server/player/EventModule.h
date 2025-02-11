#pragma once

#include "../common/Event.h"

// #include "system/event/EventParam.h"
#include "system/event/EventSystem.h"

#include <queue>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <spdlog/spdlog.h>
#include <asio.hpp>


class EventModule final {

    class Player *owner_;

    struct EventNode {
        Event event = Event::UNAVAILABLE;
        IEventParam *param = nullptr;
    };

    std::queue<EventNode> queue_;
    mutable std::shared_mutex event_mutex_;

    std::map<Event, std::map<void *, EventListener>> listener_map_;
    std::map<void *, EventListener> cur_listener_;
    std::mutex listener_mutex_;

public:
    EventModule() = delete;

    explicit EventModule(Player *plr);
    ~EventModule();

    [[nodiscard]] Player *GetOwner() const;

    [[nodiscard]] bool IsQueueEmpty() const;

    template<typename Target, typename Callable>
    void RegisterListenerT(const Event event, void *ptr, void *target, Callable && func) {
        if (event == Event::UNAVAILABLE || ptr == nullptr || target == nullptr)
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

    void RegisterListener(Event event, void *ptr, const EventListener &listener);
    void RemoveListener(Event event, void *ptr);

    void Dispatch(Event event, IEventParam *param, DispatchType type = DispatchType::PUSH_QUEUE);

private:
    asio::awaitable<void> HandleEvent();
};
