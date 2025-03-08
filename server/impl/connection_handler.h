#pragma once

#include <connection_handler.h>


class ConnectionHandler final : public IConnectionHandler {
public:
    explicit ConnectionHandler(const std::weak_ptr<UConnection> &conn);

    void onConnected() override;
    void onClosed() override;
    awaitable<void> onReadPackage(IPackage *pkg) override;
};
