#pragma once

#include <connection_handler.h>


class UConnectionHandler final : public IConnectionHandler {
public:
    explicit UConnectionHandler(const std::weak_ptr<UConnection> &conn);

    void onConnected() override;
    void onClosed() override;
    awaitable<void> onReadPackage(IPackage *pkg) override;
};
