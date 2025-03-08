#pragma once

#include "server.h"
#include "server_logic.h"


class SERVER_API GameServer final : public IServerLogic {
public:
    explicit GameServer(UGameWorld *world);
    ~GameServer() override;

    void initGameWorld() override;

    void setConnectionHandler(const std::shared_ptr<class UConnection> &conn) override;
};

