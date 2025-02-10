#pragma once

#include "../common.h"
#include "../ThreadSafeDeque.h"

#include <memory>
#include <set>
#include <map>
#include <vector>


class UReactor;
class UTaskQueue;

struct FWeakPointerRawAddressCompare {
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

    std::shared_ptr<UTaskQueue> RegisterReactor(UReactor *reactor);
    std::shared_ptr<UTaskQueue> FindByReactor(const UReactor *reactor) const;

    void RemoveReactor(const UReactor *reactor);

    void OnPushTask(const std::shared_ptr<UTaskQueue> &queue);

private:
    UGameWorld *mWorld;

    TThreadSafeDeque<std::shared_ptr<UTaskQueue>> mQueue;
    std::vector<std::thread> mWorkerVec;

    std::map<UReactor *, std::weak_ptr<UTaskQueue>> mReactorMap;
    std::mutex mReactorMutex;
    mutable std::shared_mutex mReactorShared;

    std::set<std::weak_ptr<UTaskQueue>, FWeakPointerRawAddressCompare> mEmptySet;
    std::mutex mEmptyMutex;
    mutable std::shared_mutex mEmptyShared;
};
