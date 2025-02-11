#include "GameServer.h"

#include "GameWorld.h"
#include "Connection.h"
#include "ConfigManager.h"
#include "LoginAuthenticator.h"
#include "ProtocolRoute.h"

#include "impl/ConnectionHandlerImpl.h"
#include "impl/LoginHandlerImpl.h"
#include "impl/ProtocolHandlerImpl.h"

#include "common/proto.def.h"
#include "common/config.def.h"
#include "common/logger.def.h"
#include "common/manager.def.h"


UGameServer::UGameServer(GameWorld *world)
    : IServerLogic(world) {
}

UGameServer::~UGameServer() {
}

void UGameServer::InitGameWorld() {
#ifdef WIN32
    GetWorld()->GetConfigManager()->SetYAMLPath("../../config");
#else
    GetWorld()->GetConfigManager()->SetYAMLPath("../config");
#endif

    GetWorld()->GetConfigManager()->SetLogicConfigLoader(&LoadLogicConfig);
    GetWorld()->GetConfigManager()->SetLoggerLoader(&InitLogger);

    LoadProtocol(GetWorld()->GetProtocolRoute());
    GetWorld()->GetProtocolRoute()->SetHandler<UProtocolHandlerImpl>();

    GetWorld()->GetLoginAuthenticator()->SetHandler<ULoginHandlerImpl>();

    if (const auto sys = GetWorld()->GetSystem<ManagerSystem>(); sys != nullptr)
        RegisterManager(sys);
}

void UGameServer::SetConnectionHandler(const std::shared_ptr<Connection> &conn) {
    if (conn != nullptr)
        conn->SetHandler<UConnectionHandlerImpl>();
}


extern "C" SERVER_API IServerLogic *CreateServer(GameWorld *world) {
    return new UGameServer(world);
}

extern "C" SERVER_API void DestroyServer(IServerLogic *server) {
    delete server;
}