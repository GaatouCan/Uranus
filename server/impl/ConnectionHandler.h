#pragma once

#include <ConnectionHandler.h>


class ConnectionHandler final : public IConnectionHandler {
public:
    explicit ConnectionHandler(Connection *conn);

    void OnConnected() override;
    void OnClosed() override;
    awaitable<void> OnReadPackage(IPackage *pkg) override;
};
