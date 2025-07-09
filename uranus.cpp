#include <Server.h>

#include <Config/Config.h>
#include <Login/LoginAuth.h>
#include <Event/EventModule.h>
#include <Timer/TimerModule.h>
#include <Service/ServiceModule.h>
#include <Gateway/Gateway.h>
#include <Network/Network.h>
#include <Monitor/Monitor.h>
#include <Database/DataAccess.h>
#include <Handler.h>

#include <spdlog/spdlog.h>


int main() {
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    const auto server = new UServer();

    server->SetServerHandler<UServerHandler>();

    if (auto *config = server->CreateModule<UConfig>(); config != nullptr) {
        config->SetYAMLPath("../../config");
        config->SetJSONPath("../../config");
    }

    server->CreateModule<ULoginAuth>();
    server->CreateModule<UEventModule>();
    server->CreateModule<UTimerModule>();
    server->CreateModule<UMonitor>();
    // server->CreateModule<UDataAccess>();
    server->CreateModule<UServiceModule>();
    server->CreateModule<UGateway>();
    server->CreateModule<UNetwork>();

    server->Initial();
    server->Run();

    server->Shutdown();

    delete server;
    spdlog::drop_all();

    return 0;
}
