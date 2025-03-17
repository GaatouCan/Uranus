#include "game_server.h"

#include "game_world.h"
#include "connection.h"
#include "config_manager.h"
#include "login_authenticator.h"
#include "proto_route.h"

#include "impl/connection_handler.h"
#include "impl/login_handler.h"
#include "impl/proto_handler.h"

#include "common/proto.def.h"
#include "common/config.def.h"
#include "common/logger.def.h"
#include "common/manager.def.h"


UGameServer::UGameServer(UGameWorld *world)
    : IServerLogic(world) {
}

UGameServer::~UGameServer() {
}

void UGameServer::initWorld() {
#ifdef WIN32
    getWorld()->getConfigManager()->setYamlPath("../../config");
#else
    GetWorld()->GetConfigManager()->SetYAMLPath("../config");
#endif

    getWorld()->getConfigManager()->setLogicConfigLoader(&LoadLogicConfig);
    getWorld()->getConfigManager()->setLoggerLoader(&InitLogger);

    LoadProtocol(getWorld()->getProtoRoute());
    getWorld()->getProtoRoute()->setHandler<UProtoHandler>();

    getWorld()->getLoginAuthenticator()->setHandler<ULoginHandler>();

    if (const auto sys = getWorld()->getSystem<UManagerSystem>(); sys != nullptr)
        RegisterManager(sys);
}

void UGameServer::setConnectionHandler(const std::shared_ptr<UConnection> &conn) {
    if (conn != nullptr)
        conn->setHandler<UConnectionHandler>();
}


extern "C" SERVER_API IServerLogic *CreateServer(UGameWorld *world) {
    return new UGameServer(world);
}

extern "C" SERVER_API void DestroyServer(IServerLogic *server) {
    delete server;
}