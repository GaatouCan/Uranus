#include "EventModule.h"
#include "Player.h"

#include <ranges>


UEventModule::UEventModule(UPlayer *plr)
    : mOwner(plr) {
}

UEventModule::~UEventModule() {
    mListenerMap.clear();
    while (!mQueue.empty()) {
        auto &[event, param] = mQueue.front();
        mQueue.pop();

        delete param;
    }
}

UPlayer * UEventModule::GetOwner() const {
    return mOwner;
}

bool UEventModule::IsQueueEmpty() const {
    std::shared_lock lock(mSharedMutex);
    return mQueue.empty();
}

void UEventModule::RegisterListener(const EEvent event, void *ptr, const EventListener &listener) {
    if (event == EEvent::UNAVAILABLE || ptr == nullptr)
        return;

    std::scoped_lock lock(mListenerMutex);
    if (!mListenerMap.contains(event))
        mListenerMap[event] = std::map<void *, EventListener>();

    mListenerMap[event][ptr] = listener;
}

void UEventModule::RemoveListener(const EEvent event, void *ptr) {
    if (ptr == nullptr)
        return;

    std::scoped_lock lock(mListenerMutex);
    if (event == EEvent::UNAVAILABLE) {
        for (auto &val : std::views::values(mListenerMap)) {
            val.erase(ptr);
        }
    } else {
        if (const auto iter = mListenerMap.find(event); iter != mListenerMap.end()) {
            iter->second.erase(ptr);
        }
    }
}

void UEventModule::Dispatch(EEvent event, IEventParam *param, DispatchType type) {
    spdlog::debug("{} - Player[{}] dispatch event[{}]", __FUNCTION__, mOwner->GetFullID(), static_cast<uint32_t>(event));

    if (type == DispatchType::DIRECT && GetOwner()->IsSameThread()) {
        {
            std::scoped_lock lock(mListenerMutex);
            if (const auto iter = mListenerMap.find(event); iter != mListenerMap.end()) {
                mCurListener = iter->second;
            }
        }

        for (const auto& listener : std::views::values(mCurListener)) {
            std::invoke(listener, param);
        }

        delete param;
        return;
    }

    const bool empty = IsQueueEmpty();

    {
        std::scoped_lock lock(mEventMutex);
        mQueue.emplace(event, param);
    }

    if (empty)
        co_spawn(mOwner->GetConnection()->GetSocket().get_executor(), HandleEvent(), detached);
}

awaitable<void> UEventModule::HandleEvent() {
    while (!IsQueueEmpty()) {
        FEventNode node;

        {
            std::scoped_lock lock(mEventMutex);
            node = mQueue.front();
            mQueue.pop();
        }

        if (node.event == EEvent::UNAVAILABLE) {
            delete node.param;
            continue;
        }

        mCurListener.clear();

        // 拷贝一份map 减少锁的范围 可以在调用事件处理函数时修改注册map
        {
            std::scoped_lock lock(mListenerMutex);
            if (const auto iter = mListenerMap.find(node.event); iter != mListenerMap.end()) {
                mCurListener = iter->second;
            }
        }

        for (const auto& listener : std::views::values(mCurListener)) {
            std::invoke(listener, node.param);
        }

        delete node.param;
    }

    co_return;
}
