#pragma once

#include <filesystem>
#include <functional>
#include <thread>
#include <chrono>
#include <asio.hpp>
#include <map>


#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
using AModuleHandle = HMODULE;
#else
#include <dlfcn.h>
using AModuleHandle = void *;
#endif

#include "common.h"


using asio::ip::tcp;
using asio::co_spawn;
using asio::detached;
using asio::awaitable;
using asio::as_tuple_t;
using asio::use_awaitable_t;

using default_token = as_tuple_t<use_awaitable_t<> >;

using ATcpSocket = default_token::as_default_on_t<tcp::socket>;
using ATcpAcceptor = default_token::as_default_on_t<tcp::acceptor>;

using ASteadyTimer = default_token::as_default_on_t<asio::steady_timer>;
using ASystemTimer = default_token::as_default_on_t<asio::system_timer>;

using AThreadID = std::thread::id;
using ATimePoint = std::chrono::time_point<std::chrono::system_clock>;

const auto NowTimePoint = std::chrono::system_clock::now;


namespace utils {
    BASE_API void TraverseFolder(const std::string &folder, const std::function<void(const std::filesystem::directory_entry &)> &func);

    BASE_API std::string StringReplace(std::string source, char toReplace, char replacement);

    BASE_API int64_t ThreadIDToInt(AThreadID tid);

    BASE_API std::string PascalToUnderline(const std::string &src);

    BASE_API long long UnixTime();
    BASE_API long long ToUnixTime(ATimePoint point);

    BASE_API int64_t SetBit(int64_t, int32_t);
    BASE_API int64_t ClearBit(int64_t, int32_t);
    BASE_API int64_t ToggleBit(int64_t, int32_t);
    BASE_API bool CheckBit(int64_t, int32_t);

    BASE_API std::vector<std::string> SplitString(const std::string &src, char delimiter);
    BASE_API std::vector<int> SplitStringToInt(const std::string &src, char delimiter);

    /**
     * Get Day Of The Week
     * @param point Time Point, Default Now
     * @return From 0 To 6, Means Sunday(0) To StaterDay(6)
     */
    BASE_API unsigned GetDayOfWeek(ATimePoint point = NowTimePoint());
    BASE_API unsigned GetDayOfMonth(ATimePoint point = NowTimePoint());
    BASE_API int GetDayOfYear(ATimePoint point = NowTimePoint());

    /**
     * 往日不再
     * @param former 较前的时间点
     * @param latter 较后的时间点 默认当前时间点
     * @return 经过的天数 同一天为0
     */
    BASE_API int GetDaysGone(ATimePoint former, ATimePoint latter = NowTimePoint());
    BASE_API ATimePoint GetDayZeroTime(ATimePoint point = NowTimePoint());

    BASE_API bool IsSameWeek(ATimePoint former, ATimePoint latter = NowTimePoint());
    BASE_API bool IsSameMonth(ATimePoint former, ATimePoint latter = NowTimePoint());

    BASE_API int RandomDraw(const std::map<int, int> &pool);
}
