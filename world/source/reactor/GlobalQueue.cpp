#include "../../include/reactor/GlobalQueue.h"
#include "../../include/GameWorld.h"
#include "../../include/reactor/Reactor.h"
#include "../../include/reactor/TaskQueue.h"

#include <spdlog/spdlog.h>

UGlobalQueue::UGlobalQueue(UGameWorld *world)
    : mWorld(world) {
}

UGlobalQueue::~UGlobalQueue() {
    mQueue.Quit();

    for (auto &th: mWorkerVec) {
        if (th.joinable())
            th.join();
    }
}

void UGlobalQueue::Init() {
    const auto &cfg = mWorld->GetServerConfig();
    const int num = cfg["server"]["work_thread"].as<int>();

    for (int idx = 0; idx < num; idx++) {
        mWorkerVec.emplace_back([this] {
            while (mQueue.IsRunning()) {
                mQueue.Wait();

                if (!mQueue.IsRunning())
                    break;

                auto res = mQueue.PopFront();

                if (res == nullptr)
                    continue;

                if (res->IsRemoved())
                    continue;

                res->OnPopFromGlobal();
                res->OnStart();
                res->HandleTask(10000);

                if (res->IsRemoved())
                    continue;

                res->OnStop();

                if (res->IsEmpty()) {
                    std::scoped_lock lock(mEmptyMutex);
                    mEmptySet.emplace(res);
                    continue;
                }

                mQueue.PushBack(res);
                res->OnPushToGlobal();
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

    std::scoped_lock lock(mReactorMutex, mEmptyMutex);
    mReactorMap[reactor] = queue;
    mEmptySet.emplace(queue->weak_from_this());

    return queue;
}

std::shared_ptr<UTaskQueue> UGlobalQueue::FindByReactor(const UReactor *reactor) const {
    if (reactor == nullptr)
        return nullptr;

    std::shared_lock lock(mReactorShared);
    if (const auto it = mReactorMap.find(const_cast<UReactor *>(reactor)); it != mReactorMap.end())
        return it->second.expired() ? nullptr : it->second.lock();

    return nullptr;
}

void UGlobalQueue::OnPushTask(const std::shared_ptr<UTaskQueue> &queue) {
    if (queue == nullptr)
        return;

    if (queue->IsEmpty())
        return;

    {
        std::shared_lock lock(mEmptyShared);
        if (!mEmptySet.contains(queue->weak_from_this()))
            return;
    }

    {
        std::scoped_lock lock(mEmptyMutex);
        mEmptySet.erase(queue->weak_from_this());
    }

    mQueue.PushBack(queue);
    queue->OnPushToGlobal();
}

void UGlobalQueue::RemoveReactor(const UReactor *reactor) {
    const auto queue = FindByReactor(reactor);
    if (queue == nullptr)
        return;

    queue->OnRemove();

    std::scoped_lock lock(mReactorMutex, mEmptyMutex);

    mReactorMap.erase(const_cast<UReactor *>(reactor));
    mEmptySet.erase(queue->weak_from_this());
}
