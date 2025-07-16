#pragma once

#include <asio.hpp>
#include <asio/experimental/channel.hpp>
#include <chrono>
#include <memory>

class IPackage;
using DefaultToken = asio::as_tuple_t<asio::use_awaitable_t<>>;

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::io_context;

using std::make_shared;
using std::make_unique;

using std::shared_ptr;
using std::unique_ptr;


using ATcpAcceptor = DefaultToken::as_default_on_t<asio::ip::tcp::acceptor>;
using ATcpSocket = DefaultToken::as_default_on_t<asio::ip::tcp::socket>;

using ASteadyTimer = DefaultToken::as_default_on_t<asio::steady_timer>;

using APackageChannel = DefaultToken::as_default_on_t<asio::experimental::channel<void(std::error_code, shared_ptr<IPackage>)>>;

using ASteadyTimePoint = std::chrono::steady_clock::time_point;
using ASteadyDuration = std::chrono::steady_clock::duration;

using ASystemTimePoint = std::chrono::system_clock::time_point;
using ASystemDuration = std::chrono::system_clock::duration;