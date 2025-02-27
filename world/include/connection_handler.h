#pragma once

#include "common.h"

#include <asio/awaitable.hpp>


using asio::awaitable;

class IPackage;
class Connection;


class BASE_API IConnectionHandler {

public:
    IConnectionHandler() = delete;

    explicit IConnectionHandler(const std::weak_ptr<Connection> &conn);
    virtual ~IConnectionHandler() = default;

    virtual void OnConnected() {}

    virtual awaitable<void> OnReadPackage(IPackage *) { co_return; }
    virtual awaitable<void> OnWritePackage(IPackage *) { co_return; }

    virtual void OnClosed() {}

    [[nodiscard]] class GameWorld *GetWorld() const;

protected:
    std::weak_ptr<class Connection> conn_;
};
