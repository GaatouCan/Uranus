#pragma once

#include "server.h"
#include "server_logic.h"


class SERVER_API UGameServer final : public IServerLogic {
public:
    explicit UGameServer(UGameWorld *world);
    ~UGameServer() override;

    void initGameWorld() override;

    void setConnectionHandler(const std::shared_ptr<UConnection> &conn) override;
};

