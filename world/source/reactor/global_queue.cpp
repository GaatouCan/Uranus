#include "../../include/reactor/global_queue.h"
#include "../../include/game_world.h"
#include "../../include/reactor/reactor.h"
#include "../../include/reactor/task_queue.h"

#include <spdlog/spdlog.h>

UGlobalQueue::UGlobalQueue(UGameWorld *world)
    : world_(world) {
}

UGlobalQueue::~UGlobalQueue() {
    spdlog::info("{} - Shutdown.", __FUNCTION__);
    queue_.quit();

    for (auto &th: threads_) {
        if (th.joinable())
            th.join();
    }
}

void UGlobalQueue::init() {
    const auto &cfg = world_->getServerConfig();
    const int num = cfg["server"]["work_thread"].as<int>();

    for (int idx = 0; idx < num; idx++) {
        threads_.emplace_back([this] {
            while (queue_.running()) {
                queue_.wait();

                if (!queue_.running())
                    break;

                auto res = queue_.popFront();
                if (!res.has_value()) continue;

                auto task_queue = res.value();
                if (task_queue == nullptr)
                    continue;

                if (task_queue->removed())
                    continue;

                task_queue->onPopFromGlobal();
                task_queue->onStart();
                task_queue->handleTask(10000);

                if (task_queue->removed())
                    continue;

                task_queue->onStop();

                if (task_queue->empty()) {
                    std::unique_lock lock(emptyMutex_);
                    emptySet_.emplace(task_queue);
                    continue;
                }

                queue_.pushBack(task_queue);
                task_queue->onPushToGlobal();
            }
        });
    }

    spdlog::info("Global Queue Work With {} Thread(s).", num);
}

std::shared_ptr<UTaskQueue> UGlobalQueue::registerReactor(IReactor *reactor) {
    if (reactor == nullptr)
        return nullptr;

    if (const auto res = findByReactor(reactor); res != nullptr)
        return res;

    auto queue = std::make_shared<UTaskQueue>(this, reactor);
    reactor->setTaskQueue(queue);

    std::scoped_lock lock(reactorMutex_, emptyMutex_);
    reactorMap_[reactor] = queue;
    emptySet_.emplace(queue->weak_from_this());

    return queue;
}

std::shared_ptr<UTaskQueue> UGlobalQueue::findByReactor(const IReactor *reactor) const {
    if (reactor == nullptr)
        return nullptr;

    std::shared_lock lock(reactorMutex_);
    if (const auto it = reactorMap_.find(const_cast<IReactor *>(reactor)); it != reactorMap_.end())
        return it->second.expired() ? nullptr : it->second.lock();

    return nullptr;
}

void UGlobalQueue::onPushTask(const std::shared_ptr<UTaskQueue> &queue) {
    if (queue == nullptr)
        return;

    if (queue->empty())
        return;

    {
        std::shared_lock lock(emptyMutex_);
        if (!emptySet_.contains(queue->weak_from_this()))
            return;
    }

    {
        std::unique_lock lock(emptyMutex_);
        emptySet_.erase(queue->weak_from_this());
    }

    queue_.pushBack(queue);
    queue->onPushToGlobal();
}

void UGlobalQueue::removeReactor(const IReactor *reactor) {
    const auto queue = findByReactor(reactor);
    if (queue == nullptr)
        return;

    queue->onRemove();

    std::scoped_lock lock(reactorMutex_, emptyMutex_);

    reactorMap_.erase(const_cast<IReactor *>(reactor));
    emptySet_.erase(queue->weak_from_this());
}
