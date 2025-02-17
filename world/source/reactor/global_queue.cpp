#include "../../include/reactor/global_queue.h"
#include "../../include/game_world.h"
#include "../../include/reactor/reactor.h"
#include "../../include/reactor/task_queue.h"

#include <spdlog/spdlog.h>

GlobalQueue::GlobalQueue(GameWorld *world)
    : mWorld(world) {
}

GlobalQueue::~GlobalQueue() {
    spdlog::info("{} - Shutdown.", __FUNCTION__);
    mQueue.Quit();

    for (auto &th: mWorkerVec) {
        if (th.joinable())
            th.join();
    }
}

void GlobalQueue::Init() {
    const auto &cfg = mWorld->GetServerConfig();
    const int num = cfg["server"]["work_thread"].as<int>();

    for (int idx = 0; idx < num; idx++) {
        mWorkerVec.emplace_back([this] {
            while (mQueue.IsRunning()) {
                mQueue.Wait();

                if (!mQueue.IsRunning())
                    break;

                auto res = mQueue.PopFront();
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
                    std::unique_lock lock(mEmptyMutex);
                    mEmptySet.emplace(reactor);
                    continue;
                }

                mQueue.PushBack(reactor);
                reactor->OnPushToGlobal();
            }
        });
    }

    spdlog::info("Global Queue Work With {} Thread(s).", num);
}

std::shared_ptr<TaskQueue> GlobalQueue::RegisterReactor(IReactor *reactor) {
    if (reactor == nullptr)
        return nullptr;

    if (const auto res = FindByReactor(reactor); res != nullptr)
        return res;

    auto queue = std::make_shared<TaskQueue>(this, reactor);
    reactor->SetTaskQueue(queue);

    std::scoped_lock lock(mReactorMutex, mEmptyMutex);
    mReactorMap[reactor] = queue;
    mEmptySet.emplace(queue->weak_from_this());

    return queue;
}

std::shared_ptr<TaskQueue> GlobalQueue::FindByReactor(const IReactor *reactor) const {
    if (reactor == nullptr)
        return nullptr;

    std::shared_lock lock(mReactorMutex);
    if (const auto it = mReactorMap.find(const_cast<IReactor *>(reactor)); it != mReactorMap.end())
        return it->second.expired() ? nullptr : it->second.lock();

    return nullptr;
}

void GlobalQueue::OnPushTask(const std::shared_ptr<TaskQueue> &queue) {
    if (queue == nullptr)
        return;

    if (queue->IsEmpty())
        return;

    {
        std::shared_lock lock(mEmptyMutex);
        if (!mEmptySet.contains(queue->weak_from_this()))
            return;
    }

    {
        std::unique_lock lock(mEmptyMutex);
        mEmptySet.erase(queue->weak_from_this());
    }

    mQueue.PushBack(queue);
    queue->OnPushToGlobal();
}

void GlobalQueue::RemoveReactor(const IReactor *reactor) {
    const auto queue = FindByReactor(reactor);
    if (queue == nullptr)
        return;

    queue->OnRemove();

    std::scoped_lock lock(mReactorMutex, mEmptyMutex);

    mReactorMap.erase(const_cast<IReactor *>(reactor));
    mEmptySet.erase(queue->weak_from_this());
}
