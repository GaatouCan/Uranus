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

    class Player *mOwner;

    struct EventNode {
        Event event = Event::UNAVAILABLE;
        IEventParam *param = nullptr;
    };

    std::queue<EventNode> mQueue;
    mutable std::shared_mutex mEventMutex;

    std::map<Event, std::map<void *, EventListener>> mListenerMap;
    std::map<void *, EventListener> mCurListener;
    std::mutex mListenerMutex;

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
