#pragma once

#include <connection_handler.h>


class ConnectionHandler final : public IConnectionHandler {
public:
    explicit ConnectionHandler(const std::weak_ptr<UConnection> &conn);

    void OnConnected() override;
    void OnClosed() override;
    awaitable<void> OnReadPackage(IPackage *pkg) override;
};
