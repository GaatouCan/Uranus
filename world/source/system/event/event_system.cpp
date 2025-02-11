#include "../../../include/system/event/event_system.h"
#include "../../../include/game_world.h"

#include <ranges>

EventSystem::EventSystem(GameWorld *world)
    : ISubSystem(world) {
}

EventSystem::~EventSystem() {
    mListenerMap.clear();

    while (!mEventQueue.empty()) {
        auto &[event, param] = mEventQueue.front();
        mEventQueue.pop();
        delete param;
    }
}

void EventSystem::Init() {
}


awaitable<void> EventSystem::HandleEvent() {
    while (!IsQueueEmpty()) {
        EventNode node;

        {
            std::unique_lock lock(mEventMutex);
            node = mEventQueue.front();
            mEventQueue.pop();
        }

        if (node.event == 0) {
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

        spdlog::info("{} - Event Type: {}", __FUNCTION__, node.event);

        for (const auto &listener: mCurListener | std::views::values) {
            std::invoke(listener, node.param);
        }

        delete node.param;
    }

    co_return;
}

bool EventSystem::IsQueueEmpty() const {
    std::shared_lock lock(mEventMutex);
    return mEventQueue.empty();
}

void EventSystem::Dispatch(const uint32_t event, IEventParam *param, const DispatchType type) {
   if (type == DispatchType::DIRECT && GetWorld()->IsMainThread()) {
       {
           std::scoped_lock lock(mListenerMutex);
           if (const auto iter = mListenerMap.find(event); iter != mListenerMap.end()) {
               mCurListener = iter->second;
           }
       }

       for (const auto &listener: mCurListener | std::views::values) {
           std::invoke(listener, param);
       }

       delete param;
       return;
   }

    const bool empty = IsQueueEmpty();

    {
       std::unique_lock lock(mEventMutex);
       mEventQueue.emplace(event, param);
    }

    if (empty)
        co_spawn(GetWorld()->GetIOContext(), HandleEvent(), detached);
}

void EventSystem::RegisterListener(const uint32_t event, void *ptr, const EventListener &listener) {
    if (event == 0 || ptr == nullptr)
        return;

    std::scoped_lock lock(mListenerMutex);
    if (!mListenerMap.contains(event))
        mListenerMap[event] = std::map<void *, EventListener>();

    mListenerMap[event][ptr] = listener;
}

void EventSystem::RemoveListener(const uint32_t event, void *ptr) {
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
