#pragma once

#include "common.h"

#include <asio.hpp>
#include <memory>

class ULoginAuth;
class UConnection;
class IRecycler;

class BASE_API IServerHandler {
public:
    IServerHandler() = default;
    virtual ~IServerHandler() = default;

    DISABLE_COPY_MOVE(IServerHandler)

    virtual void InitLoginAuth(ULoginAuth *auth) = 0;
    virtual void InitConnection(const std::shared_ptr<UConnection> &conn) = 0;

    virtual std::shared_ptr<IRecycler> CreatePackagePool(asio::io_context &ctx) = 0;
};
