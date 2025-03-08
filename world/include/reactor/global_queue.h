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

    void init();

    std::shared_ptr<UTaskQueue> registerReactor(IReactor *reactor);
    std::shared_ptr<UTaskQueue> findByReactor(const IReactor *reactor) const;

    void removeReactor(const IReactor *reactor);

    void onPushTask(const std::shared_ptr<UTaskQueue> &queue);

private:
    UGameWorld *world_;

    TDeque<std::shared_ptr<UTaskQueue>> queue_;
    std::vector<std::thread> threads_;

    std::map<IReactor *, std::weak_ptr<UTaskQueue>> reactorMap_;
    mutable std::shared_mutex reactorMutex_;

    std::set<std::weak_ptr<UTaskQueue>, FWeakPointerCompare> emptySet_;
    mutable std::shared_mutex emptyMutex_;
};
