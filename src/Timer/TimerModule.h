#pragma once

#include "Module.h"
#include "Utils.h"

#include <functional>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <queue>
#include <shared_mutex>


class IService;

class BASE_API UTimerModule final : public IModule {

    DECLARE_MODULE(UTimerModule)

    using ATimerTask = std::function<void(IService *)>;

    struct FTimerNode final {
        int32_t sid;
        int64_t pid;
        shared_ptr<ASteadyTimer> timer;
    };

protected:
    explicit UTimerModule(UServer *server);

    void Stop() override;

public:
    ~UTimerModule() override;

    constexpr const char *GetModuleName() const override {
        return "Timer Module";
    }

    int64_t SetTimer(int32_t sid, int64_t pid, const ATimerTask &task, int delay, int rate = -1);

    void CancelTimer(int64_t id);

    void CancelServiceTimer(int32_t sid);
    void CancelPlayerTimer(int64_t pid);

private:
    int64_t AllocateTimerID();
    void RecycleTimerID(int64_t id);

    void RemoveTimer(int64_t id);

private:
    std::queue<int64_t> mRecycledID;
    int64_t mNextID;
    mutable std::shared_mutex mIDMutex;

    absl::flat_hash_map<int64_t, FTimerNode> mTimerMap;

    absl::flat_hash_map<int32_t, absl::flat_hash_set<int64_t>> mServiceToTimer;
    absl::flat_hash_map<int64_t, absl::flat_hash_set<int64_t>> mPlayerToTimer;

    mutable std::shared_mutex mTimerMutex;
};

