#pragma once

#include "../common.h"
#include "../thread_safe_deque.h"

#include <memory>
#include <set>
#include <map>
#include <vector>


class IReactor;
class TaskQueue;
class GameWorld;

struct WeakPointerRawAddressCompare {
    template <typename T>
    bool operator()(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
        return lhs.lock().get() < rhs.lock().get();
    }
};

class GlobalQueue final {

    friend class GameWorld;

    explicit GlobalQueue(GameWorld *world);
    ~GlobalQueue();

public:
    GlobalQueue() = delete;

    DISABLE_COPY_MOVE(GlobalQueue)

    void Init();

    std::shared_ptr<TaskQueue> RegisterReactor(IReactor *reactor);
    std::shared_ptr<TaskQueue> FindByReactor(const IReactor *reactor) const;

    void RemoveReactor(const IReactor *reactor);

    void OnPushTask(const std::shared_ptr<TaskQueue> &queue);

private:
    GameWorld *mWorld;

    ThreadSafeDeque<std::shared_ptr<TaskQueue>> mQueue;
    std::vector<std::thread> mWorkerVec;

    std::map<IReactor *, std::weak_ptr<TaskQueue>> mReactorMap;
    mutable std::shared_mutex mReactorMutex;

    std::set<std::weak_ptr<TaskQueue>, WeakPointerRawAddressCompare> mEmptySet;
    mutable std::shared_mutex mEmptyMutex;
};
