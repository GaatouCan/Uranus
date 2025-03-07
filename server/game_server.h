#pragma once

#include "server.h"
#include "server_logic.h"


class SERVER_API GameServer final : public IServerLogic {
public:
    explicit GameServer(UGameWorld *world);
    ~GameServer() override;

    void InitGameWorld() override;

    void SetConnectionHandler(const std::shared_ptr<class UConnection> &conn) override;
};

