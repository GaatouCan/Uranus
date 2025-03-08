#include "event_module.h"
#include "player.h"

#include <ranges>


UEventModule::UEventModule(UPlayer *plr)
    : owner_(plr) {
}

UEventModule::~UEventModule() {
    listenerMap_.clear();
    while (!queue_.empty()) {
        auto &[event, param] = queue_.front();
        queue_.pop();

        delete param;
    }
}

UPlayer * UEventModule::getOwner() const {
    return owner_;
}

bool UEventModule::queueEmpty() const {
    std::shared_lock lock(eventMutex_);
    return queue_.empty();
}

void UEventModule::registerListener(const EEvent event, void *ptr, const AEventListener &listener) {
    if (event == EEvent::UNAVAILABLE || ptr == nullptr)
        return;

    std::scoped_lock lock(listenerMutex_);
    if (!listenerMap_.contains(event))
        listenerMap_[event] = std::map<void *, AEventListener>();

    listenerMap_[event][ptr] = listener;
}

void UEventModule::removeListener(const EEvent event, void *ptr) {
    if (ptr == nullptr)
        return;

    std::scoped_lock lock(listenerMutex_);
    if (event == EEvent::UNAVAILABLE) {
        for (auto &val : std::views::values(listenerMap_)) {
            val.erase(ptr);
        }
    } else {
        if (const auto iter = listenerMap_.find(event); iter != listenerMap_.end()) {
            iter->second.erase(ptr);
        }
    }
}

void UEventModule::dispatch(EEvent event, IEventParam *param, EDispatchType type) {
    spdlog::debug("{} - Player[{}] dispatch event[{}]", __FUNCTION__, owner_->getFullID(), static_cast<uint32_t>(event));

    if (type == EDispatchType::DIRECT && getOwner()->isSameThread()) {
        {
            std::scoped_lock lock(listenerMutex_);
            if (const auto iter = listenerMap_.find(event); iter != listenerMap_.end()) {
                curListener_ = iter->second;
            }
        }

        for (const auto& listener : std::views::values(curListener_)) {
            std::invoke(listener, param);
        }

        delete param;
        return;
    }

    const bool empty = queueEmpty();

    {
        std::unique_lock lock(eventMutex_);
        queue_.emplace(event, param);
    }

    if (empty)
        co_spawn(owner_->getIOContext(), handleEvent(), detached);
}

awaitable<void> UEventModule::handleEvent() {
    while (!queueEmpty()) {
        FEventNode node;

        {
            std::unique_lock lock(eventMutex_);
            node = queue_.front();
            queue_.pop();
        }

        if (node.event == EEvent::UNAVAILABLE) {
            delete node.param;
            continue;
        }

        curListener_.clear();

        // 拷贝一份map 减少锁的范围 可以在调用事件处理函数时修改注册map
        {
            std::scoped_lock lock(listenerMutex_);
            if (const auto iter = listenerMap_.find(node.event); iter != listenerMap_.end()) {
                curListener_ = iter->second;
            }
        }

        for (const auto& listener : std::views::values(curListener_)) {
            std::invoke(listener, node.param);
        }

        delete node.param;
    }

    co_return;
}
