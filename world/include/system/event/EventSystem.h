#pragma once

#include "../../SubSystem.h"
#include "EventParam.h"

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
    // std::mutex mEventMutex;
    mutable std::shared_mutex eventMutex_;

    using AListenerMap = std::map<void *, AEventListener>;

    std::map<uint32_t, AListenerMap> listenerMap_;
    AListenerMap currentListener_;
    std::mutex listenerMutex_;

public:
    explicit UEventSystem(UGameWorld *world);
    ~UEventSystem() override;

    GET_SYSTEM_NAME(UEventSystem)

    void Init() override;

    void Dispatch(uint32_t event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);

    template<typename TARGET, typename CALLABLE>
    void RegisterListenerT(const uint32_t event, void *ptr, void *target, CALLABLE &&func) {
        if (ptr == nullptr || target == nullptr)
            return;

        this->RegisterListener(event, ptr, [target, func = std::forward<CALLABLE>(func)](IEventParam *param) {
            try {
                if (target != nullptr) {
                    std::invoke(func, static_cast<TARGET *>(target), param);
                }
            } catch (std::exception &e) {
                spdlog::warn("Event Listener - {}", e.what());
            }
        });
    }

    void RegisterListener(uint32_t event, void *ptr, const AEventListener &listener);

    void RemoveListener(uint32_t event, void *ptr);

private:
    awaitable<void> HandleEvent();

    [[nodiscard]] bool IsQueueEmpty() const;
};
