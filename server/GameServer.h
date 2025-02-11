#pragma once

#include "server.h"
#include "ServerLogic.h"

class SERVER_API UGameServer final : public IServerLogic {
public:
    explicit UGameServer(GameWorld *world);
    ~UGameServer() override;

    void InitGameWorld() override;

    void SetConnectionHandler(const std::shared_ptr<class Connection> &conn) override;
};

