#include "../../../include/system/event/EventSystem.h"
#include "../../../include/GameWorld.h"

#include <ranges>

UEventSystem::UEventSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UEventSystem::~UEventSystem() {
    mListenerMap.clear();

    while (!mEventQueue.empty()) {
        auto &[event, param] = mEventQueue.front();
        mEventQueue.pop();
        delete param;
    }
}

void UEventSystem::Init() {
}


awaitable<void> UEventSystem::HandleEvent() {
    while (!IsQueueEmpty()) {
        FEventNode node;

        {
            std::scoped_lock lock(mEventMutex);
            node = mEventQueue.front();
            mEventQueue.pop();
        }

        if (node.event == 0) {
            delete node.param;
            continue;
        }

        mCurrentListener.clear();

        // 拷贝一份map 减少锁的范围 可以在调用事件处理函数时修改注册map
        {
            std::scoped_lock lock(mListenerMutex);
            if (const auto iter = mListenerMap.find(node.event); iter != mListenerMap.end()) {
                mCurrentListener = iter->second;
            }
        }

        for (const auto &listener: mCurrentListener | std::views::values) {
            std::invoke(listener, node.param);
        }

        delete node.param;
    }

    co_return;
}

bool UEventSystem::IsQueueEmpty() const {
    std::shared_lock lock(mEventShared);
    return mEventQueue.empty();
}

void UEventSystem::Dispatch(const uint32_t event, IEventParam *param, const EDispatchType type) {
   if (type == EDispatchType::DIRECT && GetWorld()->IsMainThread()) {
       {
           std::scoped_lock lock(mListenerMutex);
           if (const auto iter = mListenerMap.find(event); iter != mListenerMap.end()) {
               mCurrentListener = iter->second;
           }
       }

       for (const auto &listener: mCurrentListener | std::views::values) {
           std::invoke(listener, param);
       }

       delete param;
       return;
   }

    const bool empty = IsQueueEmpty();

    {
       std::scoped_lock lock(mEventMutex);
       mEventQueue.emplace(event, param);
    }

    if (empty)
        co_spawn(GetWorld()->GetIOContext(), HandleEvent(), detached);
}

void UEventSystem::RegisterListener(const uint32_t event, void *ptr, const AEventListener &listener) {
    if (event == 0 || ptr == nullptr)
        return;

    std::scoped_lock lock(mListenerMutex);
    if (!mListenerMap.contains(event))
        mListenerMap[event] = std::map<void *, AEventListener>();

    mListenerMap[event][ptr] = listener;
}

void UEventSystem::RemoveListener(const uint32_t event, void *ptr) {
    if (ptr == nullptr)
        return;

    std::scoped_lock lock(mListenerMutex);
    if (event == 0) {
        for (auto &val: mListenerMap | std::views::values)
            val.erase(ptr);
    }
    else {
        if (const auto iter = mListenerMap.find(event); iter != mListenerMap.end()) {
            iter->second.erase(ptr);
        }
    }
}
