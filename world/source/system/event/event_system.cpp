#include "../../../include/system/event/event_system.h"
#include "../../../include/game_world.h"

#include <ranges>

UEventSystem::UEventSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UEventSystem::~UEventSystem() {
    listener_map_.clear();

    while (!event_queue_.empty()) {
        auto &[event, param] = event_queue_.front();
        event_queue_.pop();
        delete param;
    }
}

void UEventSystem::Init() {
}


awaitable<void> UEventSystem::HandleEvent() {
    while (!IsQueueEmpty()) {
        EventNode node;

        {
            std::unique_lock lock(event_mtx_);
            node = event_queue_.front();
            event_queue_.pop();
        }

        if (node.event == 0) {
            delete node.param;
            continue;
        }

        cur_listener_.clear();

        // 拷贝一份map 减少锁的范围 可以在调用事件处理函数时修改注册map
        {
            std::scoped_lock lock(listener_mtx_);
            if (const auto iter = listener_map_.find(node.event); iter != listener_map_.end()) {
                cur_listener_ = iter->second;
            }
        }

        spdlog::info("{} - Event Type: {}", __FUNCTION__, node.event);

        for (const auto &listener: cur_listener_ | std::views::values) {
            std::invoke(listener, node.param);
        }

        delete node.param;
    }

    co_return;
}

bool UEventSystem::IsQueueEmpty() const {
    std::shared_lock lock(event_mtx_);
    return event_queue_.empty();
}

void UEventSystem::Dispatch(const uint32_t event, IEventParam *param, const EDispatchType type) {
   if (type == EDispatchType::DIRECT && GetWorld()->IsMainThread()) {
       {
           std::scoped_lock lock(listener_mtx_);
           if (const auto iter = listener_map_.find(event); iter != listener_map_.end()) {
               cur_listener_ = iter->second;
           }
       }

       for (const auto &listener: cur_listener_ | std::views::values) {
           std::invoke(listener, param);
       }

       delete param;
       return;
   }

    const bool empty = IsQueueEmpty();

    {
       std::unique_lock lock(event_mtx_);
       event_queue_.emplace(event, param);
    }

    if (empty)
        co_spawn(GetWorld()->GetIOContext(), HandleEvent(), detached);
}

void UEventSystem::RegisterListener(const uint32_t event, void *ptr, const EventListener &listener) {
    if (event == 0 || ptr == nullptr)
        return;

    std::scoped_lock lock(listener_mtx_);
    if (!listener_map_.contains(event))
        listener_map_[event] = std::map<void *, EventListener>();

    listener_map_[event][ptr] = listener;
}

void UEventSystem::RemoveListener(const uint32_t event, void *ptr) {
    if (ptr == nullptr)
        return;

    std::scoped_lock lock(listener_mtx_);
    if (event == 0) {
        for (auto &val: listener_map_ | std::views::values)
            val.erase(ptr);
    }
    else {
        if (const auto iter = listener_map_.find(event); iter != listener_map_.end()) {
            iter->second.erase(ptr);
        }
    }
}
