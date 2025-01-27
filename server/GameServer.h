#pragma once

#include "server.h"
#include "ServerLogic.h"

class SERVER_API UGameServer final : public IServerLogic {
public:
    explicit UGameServer(UGameWorld *world);
    ~UGameServer() override;

    void InitGameWorld() override;

    void SetConnectionHandler(const std::shared_ptr<class UConnection> &conn) override;
};

