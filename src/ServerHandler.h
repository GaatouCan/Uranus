#pragma once

#include "Common.h"

#include <asio.hpp>
#include <memory>

class ULoginAuth;
class UConnection;
class IRecyclerBase;

class BASE_API IServerHandler {
public:
    IServerHandler() = default;
    virtual ~IServerHandler() = default;

    DISABLE_COPY_MOVE(IServerHandler)

    [[nodiscard]] virtual bool IsUseCustomPackage() const {
        return false;
    }

    virtual void InitLoginAuth(ULoginAuth *auth) const = 0;
    virtual void InitConnection(const std::shared_ptr<UConnection> &conn) const = 0;

    virtual std::shared_ptr<IRecyclerBase> CreatePackagePool(asio::io_context &ctx) = 0;
};
