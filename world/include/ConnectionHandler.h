#pragma once

#include <asio/awaitable.hpp>
#include "common.h"


using asio::awaitable;

class IPackage;
class UConnection;


class BASE_API IConnectionHandler {

public:
    IConnectionHandler() = delete;

    explicit IConnectionHandler(UConnection *conn);
    virtual ~IConnectionHandler() = default;

    virtual void OnConnected() {}

    virtual awaitable<void> OnReadPackage(IPackage *) { co_return; }
    virtual awaitable<void> OnWritePackage(IPackage *) { co_return; }

    virtual void OnClosed() {}

    [[nodiscard]] class UGameWorld *GetWorld() const;

protected:
    UConnection *conn_;
};
