#pragma once

#include "../common.h"

#include <atomic>


struct BASE_API FWatchdog final {
    const int32_t sid;
    const int64_t pid;

    /** The total number of processing nodes */
    std::atomic_size_t total = 0;

    /** The number of nodes remaining to be processed */
    std::atomic_size_t rest = 0;

    /** The time point when begin process all nodes */
    std::atomic_int64_t begin = 0;

    std::atomic_int64_t end = 0;

    /** The time point when begin process last node */
    std::atomic_int64_t handle = 0;

    FWatchdog(uint32_t sid, uint64_t pid);

    void Reset();
};

