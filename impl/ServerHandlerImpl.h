#pragma once

#include "common.h"

#include <ServerHandler.h>

class IMPL_API UServerHandler final : public IServerHandler {

public:
    UServerHandler();
    ~UServerHandler() override;

    void InitLoginAuth(ULoginAuth *auth) const override;
    void InitConnection(const std::shared_ptr<UConnection> &conn) const override;

    std::shared_ptr<IRecycler> CreatePackagePool(asio::io_context &ctx) override;
};
