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


class UEventModule final {

    class UPlayer *mOwner;

    struct FEventNode {
        Event event = Event::UNAVAILABLE;
        IEventParam *param = nullptr;
    };

    std::queue<FEventNode> mQueue;
    std::mutex mEventMutex;
    mutable std::shared_mutex mSharedMutex;

    std::map<Event, std::map<void *, EventListener>> mListenerMap;
    std::map<void *, EventListener> mCurListener;
    std::mutex mListenerMutex;

public:
    UEventModule() = delete;

    explicit UEventModule(UPlayer *plr);
    ~UEventModule();

    [[nodiscard]] UPlayer *GetOwner() const;

    [[nodiscard]] bool IsQueueEmpty() const;

    template<typename TARGET, typename CALLABLE>
    void RegisterListenerT(const Event event, void *ptr, void *target, CALLABLE && func) {
        if (event == Event::UNAVAILABLE || ptr == nullptr || target == nullptr)
            return;

        this->RegisterListener(event, ptr, [target, func = std::forward<CALLABLE>(func)](IEventParam *param) {
            try {
                if (target != nullptr) {
                    std::invoke(func, static_cast<TARGET *>(target), param);
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
