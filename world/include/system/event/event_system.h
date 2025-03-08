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

enum class EDispatchType {
    PUSH_QUEUE,
    DIRECT
};

class BASE_API UEventSystem final : public ISubSystem {

    struct FEventNode {
        uint32_t event = 0;
        IEventParam *param = nullptr;
    };

    std::queue<FEventNode> eventQueue_;
    mutable std::shared_mutex eventMutex_;

    using AListenerMap = std::map<void *, AEventListener>;

    std::map<uint32_t, AListenerMap> listenerMap_;
    AListenerMap curListener_;
    std::mutex listenerMutex_;

public:
    explicit UEventSystem(UGameWorld *world);
    ~UEventSystem() override;

    GET_SYSTEM_NAME(EventSystem)

    void init() override;

    void dispatch(uint32_t event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);

    template<typename TargetType, typename Callable>
    void registerListenerT(const uint32_t event, void *ptr, void *target, Callable &&func) {
        if (ptr == nullptr || target == nullptr)
            return;

        registerListener(event, ptr, [target, func = std::forward<Callable>(func)](IEventParam *param) {
            try {
                if (target != nullptr) {
                    std::invoke(func, static_cast<TargetType *>(target), param);
                }
            } catch (std::exception &e) {
                spdlog::warn("Event Listener - {}", e.what());
            }
        });
    }

    void registerListener(uint32_t event, void *ptr, const AEventListener &listener);

    void removeListener(uint32_t event, void *ptr);

private:
    awaitable<void> handleEvent();

    [[nodiscard]] bool queueEmpty() const;
};
