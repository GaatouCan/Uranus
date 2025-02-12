#pragma once

#include <asio/awaitable.hpp>
#include "common.h"


using asio::awaitable;

class IPackage;
class Connection;


class BASE_API IConnectionHandler {

public:
    IConnectionHandler() = delete;

    explicit IConnectionHandler(Connection *conn);
    virtual ~IConnectionHandler() = default;

    virtual void OnConnected() {}

    virtual awaitable<void> OnReadPackage(IPackage *) { co_return; }
    virtual awaitable<void> OnWritePackage(IPackage *) { co_return; }

    virtual void OnClosed() {}

    [[nodiscard]] class GameWorld *GetWorld() const;

protected:
    Connection *mConn;
};
