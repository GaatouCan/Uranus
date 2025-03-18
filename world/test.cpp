#include <spdlog/spdlog.h>
#include "include/utils.h"

int main() {

    const auto dayOfYear = utils::GetDayOfMonth(NowTimePoint());
    spdlog::info("dayOfYear: {}", dayOfYear);

    return 0;
}