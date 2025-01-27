#pragma once

#include <asio/awaitable.hpp>


class IPackage;

using asio::awaitable;

class ICrossRouteHandler {

public:
    virtual ~ICrossRouteHandler() = default;

    virtual void OnConnect() {}

    virtual awaitable<void> OnReadPackage(IPackage *) { co_return; }
    virtual awaitable<void> OnWritePackage(IPackage *) { co_return; }

    virtual void OnClose() {}
};