#include "Watchdog.h"
#include "Utils.h"

FWatchdog::FWatchdog(const uint32_t sid, const uint64_t pid)
    : sid(sid),
      pid(pid) {
}

void FWatchdog::Reset() {
    total = 0;
    rest = 0;
    begin = 0;
    end = 0;
    handle = 0;
}
