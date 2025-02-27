#pragma once

#include <filesystem>
#include <functional>
#include <thread>
#include <chrono>
#include <asio.hpp>

#include "common.h"

using asio::ip::tcp;

using asio::co_spawn;
using asio::detached;
using asio::awaitable;

using asio::as_tuple_t;
using asio::use_awaitable_t;
using asio::deferred_t;

using default_token = as_tuple_t<use_awaitable_t<>>;

using TcpSocket = deferred_t::as_default_on_t<tcp::socket>;
using TcpAcceptor = deferred_t::as_default_on_t<tcp::acceptor>;

using SteadyTimer = default_token::as_default_on_t<asio::steady_timer>;
using SystemTimer = default_token::as_default_on_t<asio::system_timer>;

const auto NowTimePoint = std::chrono::system_clock::now;

using ThreadID = std::thread::id;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using Duration = std::chrono::steady_clock::duration;

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

using ModuleHandle = HMODULE;

#else

#include <dlfcn.h>

using ModuleHandle = void*;

#endif

class GameWorld;

namespace utils {

    void BASE_API SetGameWorld(GameWorld *world);
    GameWorld BASE_API *GetWorld();

    void BASE_API TraverseFolder(const std::string &folder, const std::function<void(const std::filesystem::directory_entry &)> &func);

    std::string BASE_API StringReplace(std::string source, char toReplace, char replacement);

    int64_t BASE_API ThreadIdToInt(std::thread::id threadID);

    std::string BASE_API PascalToUnderline(const std::string &src);

    long long BASE_API UnixTime();
    long long BASE_API ToUnixTime(TimePoint point);

    int64_t BASE_API SetBit(int64_t, int32_t);
    int64_t BASE_API ClearBit(int64_t, int32_t);
    int64_t BASE_API ToggleBit(int64_t, int32_t);
    bool BASE_API CheckBit(int64_t, int32_t);

    std::vector<std::string> BASE_API SplitString(const std::string &src, char delimiter);
    std::vector<int> BASE_API SplitStringToInt(const std::string &src, char delimiter);

    /**
     * Get Day Of The Week
     * @param point Time Point, Default Now
     * @return From 0 To 6, Means Sunday(0) To StaterDay(6)
     */
    int BASE_API GetDayOfWeek(TimePoint point = std::chrono::system_clock::now());
    unsigned BASE_API GetDayOfMonth(TimePoint point = std::chrono::system_clock::now());
    int BASE_API GetDayOfYear(TimePoint point = std::chrono::system_clock::now());

    /**
     * 往日不再
     * @param former 较前的时间点
     * @param latter 较后的时间点 默认当前时间点
     * @return 经过的天数 同一天为0
     */
    int BASE_API GetDaysGone(TimePoint former, TimePoint latter = std::chrono::system_clock::now());
    TimePoint BASE_API GetDayZeroTime(TimePoint point = std::chrono::system_clock::now());
}
