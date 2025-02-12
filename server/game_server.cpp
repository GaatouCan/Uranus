#include "game_server.h"

#include "game_world.h"
#include "connection.h"
#include "config_manager.h"
#include "login_authenticator.h"
#include "protocol_route.h"

#include "impl/connection_handler.h"
#include "impl/login_handler.h"
#include "impl/protocol_handler.h"

#include "common/proto.def.h"
#include "common/config.def.h"
#include "common/logger.def.h"
#include "common/manager.def.h"


GameServer::GameServer(GameWorld *world)
    : IServerLogic(world) {
}

GameServer::~GameServer() {
}

void GameServer::InitGameWorld() {
#ifdef WIN32
    GetWorld()->GetConfigManager()->SetYAMLPath("../../config");
#else
    GetWorld()->GetConfigManager()->SetYAMLPath("../config");
#endif

    GetWorld()->GetConfigManager()->SetLogicConfigLoader(&LoadLogicConfig);
    GetWorld()->GetConfigManager()->SetLoggerLoader(&InitLogger);

    LoadProtocol(GetWorld()->GetProtocolRoute());
    GetWorld()->GetProtocolRoute()->SetHandler<ProtocolHandler>();

    GetWorld()->GetLoginAuthenticator()->SetHandler<LoginHandler>();

    if (const auto sys = GetWorld()->GetSystem<ManagerSystem>(); sys != nullptr)
        RegisterManager(sys);
}

void GameServer::SetConnectionHandler(const std::shared_ptr<Connection> &conn) {
    if (conn != nullptr)
        conn->SetHandler<ConnectionHandler>();
}


extern "C" SERVER_API IServerLogic *CreateServer(GameWorld *world) {
    return new GameServer(world);
}

extern "C" SERVER_API void DestroyServer(IServerLogic *server) {
    delete server;
}