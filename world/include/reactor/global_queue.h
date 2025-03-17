#pragma once

#include "../common.h"
#include "../ts_deque.h"

#include <memory>
#include <vector>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>


class IReactor;
class UTaskQueue;
class UGameWorld;

struct FWeakPointerHash {
    using is_transparent = void;

    template<typename T>
    std::size_t operator()(const std::weak_ptr<T> &wPtr) const {
        return std::hash<T *>{}(wPtr.lock().get());
    }
};

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

    absl::flat_hash_map<IReactor *, std::weak_ptr<UTaskQueue>> reactorMap_;
    mutable std::shared_mutex reactorMutex_;

    absl::flat_hash_set<std::weak_ptr<UTaskQueue>, FWeakPointerHash, FWeakPointerCompare> emptySet_;
    mutable std::shared_mutex emptyMutex_;
};
