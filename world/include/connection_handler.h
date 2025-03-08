#pragma once

#include "common.h"

#include <asio/awaitable.hpp>


using asio::awaitable;

class IPackage;
class UConnection;


class BASE_API IConnectionHandler {

public:
    IConnectionHandler() = delete;

    explicit IConnectionHandler(const std::weak_ptr<UConnection> &conn);
    virtual ~IConnectionHandler() = default;

    virtual void onConnected() {}

    virtual awaitable<void> onReadPackage(IPackage *) { co_return; }
    virtual awaitable<void> onWritePackage(IPackage *) { co_return; }

    virtual void onClosed() {}

    [[nodiscard]] class UGameWorld *getWorld() const;

protected:
    std::weak_ptr<UConnection> conn_;
};
