#include "event_module.h"
#include "player.h"

#include <ranges>


EventModule::EventModule(Player *plr)
    : owner_(plr) {
}

EventModule::~EventModule() {
    listener_map_.clear();
    while (!queue_.empty()) {
        auto &[event, param] = queue_.front();
        queue_.pop();

        delete param;
    }
}

Player * EventModule::GetOwner() const {
    return owner_;
}

bool EventModule::IsQueueEmpty() const {
    std::shared_lock lock(event_mtx_);
    return queue_.empty();
}

void EventModule::RegisterListener(const EEvent event, void *ptr, const AEventListener &listener) {
    if (event == EEvent::UNAVAILABLE || ptr == nullptr)
        return;

    std::scoped_lock lock(listener_mtx_);
    if (!listener_map_.contains(event))
        listener_map_[event] = std::map<void *, AEventListener>();

    listener_map_[event][ptr] = listener;
}

void EventModule::RemoveListener(const EEvent event, void *ptr) {
    if (ptr == nullptr)
        return;

    std::scoped_lock lock(listener_mtx_);
    if (event == EEvent::UNAVAILABLE) {
        for (auto &val : std::views::values(listener_map_)) {
            val.erase(ptr);
        }
    } else {
        if (const auto iter = listener_map_.find(event); iter != listener_map_.end()) {
            iter->second.erase(ptr);
        }
    }
}

void EventModule::Dispatch(EEvent event, IEventParam *param, EDispatchType type) {
    spdlog::debug("{} - Player[{}] dispatch event[{}]", __FUNCTION__, owner_->getFullID(), static_cast<uint32_t>(event));

    if (type == EDispatchType::DIRECT && GetOwner()->isSameThread()) {
        {
            std::scoped_lock lock(listener_mtx_);
            if (const auto iter = listener_map_.find(event); iter != listener_map_.end()) {
                cur_listener_ = iter->second;
            }
        }

        for (const auto& listener : std::views::values(cur_listener_)) {
            std::invoke(listener, param);
        }

        delete param;
        return;
    }

    const bool empty = IsQueueEmpty();

    {
        std::unique_lock lock(event_mtx_);
        queue_.emplace(event, param);
    }

    if (empty)
        co_spawn(owner_->getIOContext(), HandleEvent(), detached);
}

awaitable<void> EventModule::HandleEvent() {
    while (!IsQueueEmpty()) {
        EventNode node;

        {
            std::unique_lock lock(event_mtx_);
            node = queue_.front();
            queue_.pop();
        }

        if (node.event == EEvent::UNAVAILABLE) {
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

        for (const auto& listener : std::views::values(cur_listener_)) {
            std::invoke(listener, node.param);
        }

        delete node.param;
    }

    co_return;
}
