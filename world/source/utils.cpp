#include "../include/utils.h"

#include <cassert>
#include <ctime>

namespace utils {

    void TraverseFolder(const std::string &folder, const std::function<void(const std::filesystem::directory_entry &)> &func) {
        for (const auto &entry: std::filesystem::directory_iterator(folder)) {
            if (entry.is_directory()) {
                TraverseFolder(entry.path().string(), func);
            } else {
                std::invoke(func, entry);
            }
        }
    }

    std::string StringReplace(std::string source, const char toReplace, const char replacement) {
        size_t pos = 0;
        while ((pos = source.find(toReplace, pos)) != std::string::npos) {
            source.replace(pos, 1, 1, replacement);
            pos += 1;
        }
        return source;
    }

    int64_t ThreadIdToInt(const std::thread::id threadID) {
        std::stringstream ss;
        ss << threadID;
        return std::stoll(ss.str());
    }

    std::string PascalToUnderline(const std::string &src) {
        if (src.empty()) return "";

        std::string result;
        result.reserve(src.size());

        for (size_t i = 0; i < src.size(); ++i) {
            if (i > 0 && isupper(src[i])) {
                result.push_back('_');
            }
            result.push_back(tolower(src[i]));
        }

        return result;
    }

    long long UnixTime() {
        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);
        return secondsSinceEpoch.count();
    }

    long long ToUnixTime(const TimePoint point) {
        const auto durationSinceEpoch = point.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);
        return secondsSinceEpoch.count();
    }

    int64_t SetBit(const int64_t number, const int32_t n) {
        assert(n >= 0 && n < sizeof(int64_t) * 8);
        return number | static_cast<int64_t>(1) << n;
    }

    int64_t ClearBit(const int64_t number, const int32_t n) {
        assert(n >= 0 && n < sizeof(int64_t) * 8);
        return number & ~(static_cast<int64_t>(1) << n);
    }

    int64_t ToggleBit(const int64_t number, const int32_t n) {
        assert(n >= 0 && n < sizeof(int64_t) * 8);
        return number ^ static_cast<int64_t>(1) << n;
    }

    bool CheckBit(const int64_t number, const int32_t n) {
        assert(n >= 0 && n < sizeof(int64_t) * 8);
        return number & static_cast<int64_t>(1) << n;
    }

    std::vector<std::string> SplitString(const std::string &src, const char delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(src);
        std::string item;

        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }

        return result;
    }


    std::vector<int> SplitStringToInt(const std::string &src, const char delimiter) {
        std::vector<int> result;
        std::stringstream ss(src);
        std::string item;

        while (std::getline(ss, item, delimiter)) {
            result.push_back(std::stoi(item));}

        return result;
    }

    int GetDayOfWeek(const TimePoint point) {
        const std::time_t currentTime = std::chrono::system_clock::to_time_t(point);
        std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm, &currentTime);
#else
        localtime_r(&currentTime, &tm);
#endif
        return tm.tm_wday;
    }

    unsigned GetDayOfMonth(const TimePoint point) {
        const auto localTime = std::chrono::current_zone()->to_local(point);
        const std::chrono::year_month_day ymd(std::chrono::floor<std::chrono::days>(localTime));
        return static_cast<unsigned>(ymd.day());
    }

    int GetDayOfYear(const TimePoint point) {
        const auto localTime = std::chrono::current_zone()->to_local(point);

        const auto today = std::chrono::floor<std::chrono::days>(localTime);

        const auto thisYear = std::chrono::floor<std::chrono::years>(localTime);
        const auto firstDayOfYear = std::chrono::floor<std::chrono::days>(thisYear);

        return today.time_since_epoch().count() - firstDayOfYear.time_since_epoch().count() + 1;
    }

    int GetDaysGone(const TimePoint former, const TimePoint latter) {
        if (latter <= former)
            return 0;

        const auto formerLocalTime = std::chrono::current_zone()->to_local(former);
        const auto latterLocalTime = std::chrono::current_zone()->to_local(latter);

        const auto formerDays = std::chrono::floor<std::chrono::days>(formerLocalTime);
        const auto latterDays = std::chrono::floor<std::chrono::days>(latterLocalTime);

        return latterDays.time_since_epoch().count() - formerDays.time_since_epoch().count();
    }

    TimePoint GetDayZeroTime(const TimePoint point) {
        const auto zeroLocalTime =  std::chrono::floor<std::chrono::days>(std::chrono::current_zone()->to_local(point));
        return std::chrono::current_zone()->to_sys(zeroLocalTime);
    }
}
