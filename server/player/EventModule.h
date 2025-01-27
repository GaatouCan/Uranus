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
        EEvent event = EEvent::UNAVAILABLE;
        IEventParam *param = nullptr;
    };

    std::queue<FEventNode> mQueue;
    std::mutex mEventMutex;
    mutable std::shared_mutex mSharedMutex;

    std::map<EEvent, std::map<void *, AEventListener>> mListenerMap;
    std::map<void *, AEventListener> mCurListener;
    std::mutex mListenerMutex;

public:
    UEventModule() = delete;

    explicit UEventModule(UPlayer *plr);
    ~UEventModule();

    [[nodiscard]] UPlayer *GetOwner() const;

    [[nodiscard]] bool IsQueueEmpty() const;

    template<typename TARGET, typename CALLABLE>
    void RegisterListenerT(const EEvent event, void *ptr, void *target, CALLABLE && func) {
        if (event == EEvent::UNAVAILABLE || ptr == nullptr || target == nullptr)
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

    void RegisterListener(EEvent event, void *ptr, const AEventListener &listener);
    void RemoveListener(EEvent event, void *ptr);

    void Dispatch(EEvent event, IEventParam *param, EDispatchType type = EDispatchType::PUSH_QUEUE);

private:
    asio::awaitable<void> HandleEvent();
};
