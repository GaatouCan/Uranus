#include <GameWorld.h>
#include <Connection.h>

#include <system/config/ConfigSystem.h>
#include <system/protocol/ProtocolSystem.h>
#include <system/login/LoginSystem.h>

#include "impl/ConnectionHandlerImpl.h"
#include "impl/ProtocolHandlerImpl.h"
#include "impl/LoginHandlerImpl.h"
#include "impl/CrossRouteHandlerImpl.h"

#include "common/config.def.h"
#include "common/proto.def.h"
#include "common/logger.def.h"
#include "common/manager.def.h"

auto main(int argc, char *argv[]) -> int {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Welcome To ClockTower!");

    // 设置服务器配置文件路径
    if (const auto sys = UConfigSystem::Instance(); sys != nullptr) {
#ifdef WIN32
        sys->SetYAMLPath("../../config");
#else
        sys->setYAMLPath("../config");
#endif
        sys->SetLogicConfigLoader(&LoadLogicConfig);
        sys->SetLoggerLoader(&InitLogger);
    }

    // 注册程式生成协议定义以及协议处理器
    if (const auto sys = UProtocolSystem::Instance(); sys != nullptr) {
        LoadProtocol(sys);
        sys->SetHandler<UProtocolHandlerImpl>();
    }

    // 设置玩家登陆请求处理器
    if (const auto sys = ULoginSystem::Instance(); sys != nullptr) {
        sys->SetHandler<ULoginHandlerImpl>();
    }

    // 注册Manager
    if (const auto sys = UManagerSystem::Instance(); sys != nullptr) {
        LoadManager(sys);
    }

    // 设置新连接处理
    UGameWorld::Instance().FilterConnection([](const AConnectionPointer &conn) {
        if (conn != nullptr) {
            conn->SetHandler<UConnectionHandlerImpl>();
        }
    });

    UCrossRoute::Instance().SetHandler<UCrossRouteHandlerImpl>();

    // 加载跨服协议
    LoadCrossProtocol();

    auto crossThread = std::thread([] {
        // UCrossRoute::Instance().ConnectToCross("localhost", "8080");
        UCrossRoute::Instance().Run();
    });

    // 启动服务器
    UGameWorld::Instance().Init();
    UGameWorld::Instance().Run();

    if (crossThread.joinable()) {
        crossThread.join();
    }

    return 0;
}