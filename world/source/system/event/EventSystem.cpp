#include "../../../include/system/event/EventSystem.h"
#include "../../../include/GameWorld.h"

#include <ranges>

UEventSystem::UEventSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UEventSystem::~UEventSystem() {
    listenerMap_.clear();

    while (!eventQueue_.empty()) {
        auto &[event, param] = eventQueue_.front();
        eventQueue_.pop();
        delete param;
    }
}

void UEventSystem::Init() {
}


awaitable<void> UEventSystem::HandleEvent() {
    while (!IsQueueEmpty()) {
        FEventNode node;

        {
            std::unique_lock lock(eventMutex_);
            node = eventQueue_.front();
            eventQueue_.pop();
        }

        if (node.event == 0) {
            delete node.param;
            continue;
        }

        currentListener_.clear();

        // 拷贝一份map 减少锁的范围 可以在调用事件处理函数时修改注册map
        {
            std::scoped_lock lock(listenerMutex_);
            if (const auto iter = listenerMap_.find(node.event); iter != listenerMap_.end()) {
                currentListener_ = iter->second;
            }
        }

        spdlog::info("{} - Event Type: {}", __FUNCTION__, node.event);

        for (const auto &listener: currentListener_ | std::views::values) {
            std::invoke(listener, node.param);
        }

        delete node.param;
    }

    co_return;
}

bool UEventSystem::IsQueueEmpty() const {
    std::shared_lock lock(eventMutex_);
    return eventQueue_.empty();
}

void UEventSystem::Dispatch(const uint32_t event, IEventParam *param, const EDispatchType type) {
   if (type == EDispatchType::DIRECT && GetWorld()->IsMainThread()) {
       {
           std::scoped_lock lock(listenerMutex_);
           if (const auto iter = listenerMap_.find(event); iter != listenerMap_.end()) {
               currentListener_ = iter->second;
           }
       }

       for (const auto &listener: currentListener_ | std::views::values) {
           std::invoke(listener, param);
       }

       delete param;
       return;
   }

    const bool empty = IsQueueEmpty();

    {
       std::unique_lock lock(eventMutex_);
       eventQueue_.emplace(event, param);
    }

    if (empty)
        co_spawn(GetWorld()->GetIOContext(), HandleEvent(), detached);
}

void UEventSystem::RegisterListener(const uint32_t event, void *ptr, const AEventListener &listener) {
    if (event == 0 || ptr == nullptr)
        return;

    std::scoped_lock lock(listenerMutex_);
    if (!listenerMap_.contains(event))
        listenerMap_[event] = std::map<void *, AEventListener>();

    listenerMap_[event][ptr] = listener;
}

void UEventSystem::RemoveListener(const uint32_t event, void *ptr) {
    if (ptr == nullptr)
        return;

    std::scoped_lock lock(listenerMutex_);
    if (event == 0) {
        for (auto &val: listenerMap_ | std::views::values)
            val.erase(ptr);
    }
    else {
        if (const auto iter = listenerMap_.find(event); iter != listenerMap_.end()) {
            iter->second.erase(ptr);
        }
    }
}
