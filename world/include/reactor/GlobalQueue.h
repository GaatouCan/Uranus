#pragma once

#include "../common.h"
#include "../ThreadSafeDeque.h"

#include <memory>
#include <set>
#include <map>
#include <vector>


class IReactor;
class TaskQueue;

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
    GameWorld *world_;

    ThreadSafeDeque<std::shared_ptr<TaskQueue>> queue_;
    std::vector<std::thread> worker_vec_;

    std::map<IReactor *, std::weak_ptr<TaskQueue>> reactor_map_;
    mutable std::shared_mutex reactor_mutex_;

    std::set<std::weak_ptr<TaskQueue>, WeakPointerRawAddressCompare> empty_set_;
    mutable std::shared_mutex empty_mutex_;
};
