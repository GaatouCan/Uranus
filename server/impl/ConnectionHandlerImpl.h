#pragma once

#include "ConnectionHandler.h"


class UConnectionHandlerImpl final : public IConnectionHandler {
public:
    explicit UConnectionHandlerImpl(Connection *conn);

    void OnConnected() override;
    void OnClosed() override;
    awaitable<void> OnReadPackage(IPackage *pkg) override;
};
