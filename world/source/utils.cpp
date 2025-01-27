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

    uint64_t ThreadIdToInt(const std::thread::id threadID) {
        std::stringstream ss;
        ss << threadID;
        return std::stoull(ss.str());
    }

    std::string PascalToUnderline(const std::string &src) {
        std::string res;

        const auto length = src.size();
        res.resize(length * 2);

        bool bFirstLetter = true;
        size_t count = 0;

        for (auto idx = 0; idx < length; idx++) {
            if (src[idx] >= 'A' && src[idx] <= 'Z' && bFirstLetter) {
                bFirstLetter = false;
                res[idx] = src[idx] - 'A' + 'a';
            } else if (src[idx] >= 'A' && src[idx] <= 'Z' && !bFirstLetter) {
                res[idx + count] = '_';
                count++;
                res[idx + count] = src[idx] - 'A' + 'a';
            } else {
                res[idx + count] = src[idx];
            }
        }

        return res;
    }

    long long UnixTime() {
        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);
        return secondsSinceEpoch.count();
    }

    long long ToUnixTime(const ATimePoint point) {
        const auto durationSinceEpoch = point.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);
        return secondsSinceEpoch.count();
    }

    uint64_t SetBit(const uint64_t number, const uint32_t n) {
        assert(n < sizeof(uint64_t) * 8);
        return number | static_cast<uint64_t>(1) << n;
    }

    uint64_t ClearBit(const uint64_t number, const uint32_t n) {
        assert(n < sizeof(uint64_t) * 8);
        return number & ~(static_cast<uint64_t>(1) << n);
    }

    uint64_t ToggleBit(const uint64_t number, const uint32_t n) {
        assert(n < sizeof(uint64_t) * 8);
        return number ^ static_cast<uint64_t>(1) << n;
    }

    bool CheckBit(const uint64_t number, const uint32_t n) {
        assert(n < sizeof(uint64_t) * 8);
        return number & static_cast<uint64_t>(1) << n;
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

    int GetDayOfWeek(const ATimePoint point) {
        const std::time_t currentTime = std::chrono::system_clock::to_time_t(point);
        std::tm tm{};
#ifdef WIN32
        localtime_s(&tm, &currentTime);
#else
        localtime_r(&currentTime, &tm);
#endif
        return tm.tm_wday;
    }

    unsigned GetDayOfMonth(const ATimePoint point) {
        const auto localTime = std::chrono::current_zone()->to_local(point);
        const std::chrono::year_month_day ymd(std::chrono::floor<std::chrono::days>(localTime));
        return static_cast<unsigned>(ymd.day());
    }

    int GetDayOfYear(const ATimePoint point) {
        const auto localTime = std::chrono::current_zone()->to_local(point);

        const auto today = std::chrono::floor<std::chrono::days>(localTime);

        const auto thisYear = std::chrono::floor<std::chrono::years>(localTime);
        const auto firstDayOfYear = std::chrono::floor<std::chrono::days>(thisYear);

        return today.time_since_epoch().count() - firstDayOfYear.time_since_epoch().count() + 1;
    }

    int GetDaysGone(const ATimePoint former, const ATimePoint latter) {
        if (latter <= former)
            return 0;

        const auto formerLocalTime = std::chrono::current_zone()->to_local(former);
        const auto latterLocalTime = std::chrono::current_zone()->to_local(latter);

        const auto formerDays = std::chrono::floor<std::chrono::days>(formerLocalTime);
        const auto latterDays = std::chrono::floor<std::chrono::days>(latterLocalTime);

        return latterDays.time_since_epoch().count() - formerDays.time_since_epoch().count();
    }

    ATimePoint GetDayZeroTime(const ATimePoint point) {
        const auto zeroLocalTime =  std::chrono::floor<std::chrono::days>(std::chrono::current_zone()->to_local(point));
        return std::chrono::current_zone()->to_sys(zeroLocalTime);
    }
}
