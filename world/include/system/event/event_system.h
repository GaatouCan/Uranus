#pragma once

#include "../../sub_system.h"
#include "event_param.h"

#include <queue>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <asio.hpp>
#include <spdlog/spdlog.h>


using asio::awaitable;

enum class DispatchType {
    PUSH_QUEUE,
    DIRECT
};

class BASE_API EventSystem final : public ISubSystem {

    struct EventNode {
        uint32_t event = 0;
        IEventParam *param = nullptr;
    };

    std::queue<EventNode> mEventQueue;
    mutable std::shared_mutex mEventMutex;

    using ListenerMap = std::map<void *, EventListener>;

    std::map<uint32_t, ListenerMap> mListenerMap;
    ListenerMap mCurListener;
    std::mutex mListenerMutex;

public:
    explicit EventSystem(GameWorld *world);
    ~EventSystem() override;

    GET_SYSTEM_NAME(UEventSystem)

    void Init() override;

    void Dispatch(uint32_t event, IEventParam *param, DispatchType type = DispatchType::PUSH_QUEUE);

    template<typename TargetType, typename Callable>
    void RegisterListenerT(const uint32_t event, void *ptr, void *target, Callable &&func) {
        if (ptr == nullptr || target == nullptr)
            return;

        this->RegisterListener(event, ptr, [target, func = std::forward<Callable>(func)](IEventParam *param) {
            try {
                if (target != nullptr) {
                    std::invoke(func, static_cast<TargetType *>(target), param);
                }
            } catch (std::exception &e) {
                spdlog::warn("Event Listener - {}", e.what());
            }
        });
    }

    void RegisterListener(uint32_t event, void *ptr, const EventListener &listener);

    void RemoveListener(uint32_t event, void *ptr);

private:
    awaitable<void> HandleEvent();

    [[nodiscard]] bool IsQueueEmpty() const;
};
