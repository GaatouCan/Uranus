#pragma once

#include "../common.h"
#include "../ts_deque.h"

#include <memory>
#include <set>
#include <map>
#include <vector>


class IReactor;
class UTaskQueue;
class UGameWorld;

struct FWeakPointerCompare {
    template <typename T>
    bool operator()(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
        return lhs.lock().get() < rhs.lock().get();
    }
};

class UGlobalQueue final {

    friend class UGameWorld;

    explicit UGlobalQueue(UGameWorld *world);
    ~UGlobalQueue();

public:
    UGlobalQueue() = delete;

    DISABLE_COPY_MOVE(UGlobalQueue)

    void Init();

    std::shared_ptr<UTaskQueue> RegisterReactor(IReactor *reactor);
    std::shared_ptr<UTaskQueue> FindByReactor(const IReactor *reactor) const;

    void RemoveReactor(const IReactor *reactor);

    void OnPushTask(const std::shared_ptr<UTaskQueue> &queue);

private:
    UGameWorld *world_;

    TDeque<std::shared_ptr<UTaskQueue>> queue_;
    std::vector<std::thread> worker_vec_;

    std::map<IReactor *, std::weak_ptr<UTaskQueue>> reactor_map_;
    mutable std::shared_mutex reactor_mtx_;

    std::set<std::weak_ptr<UTaskQueue>, FWeakPointerCompare> empty_set_;
    mutable std::shared_mutex empty_mtx_;
};
