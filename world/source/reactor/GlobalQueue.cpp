#include "../../include/reactor/GlobalQueue.h"
#include "../../include/GameWorld.h"
#include "../../include/reactor/Reactor.h"
#include "../../include/reactor/TaskQueue.h"

#include <spdlog/spdlog.h>

UGlobalQueue::UGlobalQueue(UGameWorld *world)
    : world_(world) {
}

UGlobalQueue::~UGlobalQueue() {
    queue_.Quit();

    for (auto &th: workerVec_) {
        if (th.joinable())
            th.join();
    }
}

void UGlobalQueue::Init() {
    const auto &cfg = world_->GetServerConfig();
    const int num = cfg["server"]["work_thread"].as<int>();

    for (int idx = 0; idx < num; idx++) {
        workerVec_.emplace_back([this] {
            while (queue_.IsRunning()) {
                queue_.Wait();

                if (!queue_.IsRunning())
                    break;

                auto res = queue_.PopFront();
                if (!res.has_value()) continue;

                auto reactor = res.value();
                if (reactor == nullptr)
                    continue;

                if (reactor->IsRemoved())
                    continue;

                reactor->OnPopFromGlobal();
                reactor->OnStart();
                reactor->HandleTask(10000);

                if (reactor->IsRemoved())
                    continue;

                reactor->OnStop();

                if (reactor->IsEmpty()) {
                    std::unique_lock lock(emptyMutex_);
                    emptySet_.emplace(reactor);
                    continue;
                }

                queue_.PushBack(reactor);
                reactor->OnPushToGlobal();
            }
        });
    }

    spdlog::info("Reactor System Work With {} Thread(s).", num);
}

std::shared_ptr<UTaskQueue> UGlobalQueue::RegisterReactor(UReactor *reactor) {
    if (reactor == nullptr)
        return nullptr;

    if (const auto res = FindByReactor(reactor); res != nullptr)
        return res;

    auto queue = std::make_shared<UTaskQueue>(this, reactor);
    reactor->SetTaskQueue(queue);

    std::scoped_lock lock(reactorMutex_, emptyMutex_);
    reactorMap_[reactor] = queue;
    emptySet_.emplace(queue->weak_from_this());

    return queue;
}

std::shared_ptr<UTaskQueue> UGlobalQueue::FindByReactor(const UReactor *reactor) const {
    if (reactor == nullptr)
        return nullptr;

    std::shared_lock lock(reactorMutex_);
    if (const auto it = reactorMap_.find(const_cast<UReactor *>(reactor)); it != reactorMap_.end())
        return it->second.expired() ? nullptr : it->second.lock();

    return nullptr;
}

void UGlobalQueue::OnPushTask(const std::shared_ptr<UTaskQueue> &queue) {
    if (queue == nullptr)
        return;

    if (queue->IsEmpty())
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

    queue_.PushBack(queue);
    queue->OnPushToGlobal();
}

void UGlobalQueue::RemoveReactor(const UReactor *reactor) {
    const auto queue = FindByReactor(reactor);
    if (queue == nullptr)
        return;

    queue->OnRemove();

    std::scoped_lock lock(reactorMutex_, emptyMutex_);

    reactorMap_.erase(const_cast<UReactor *>(reactor));
    emptySet_.erase(queue->weak_from_this());
}
