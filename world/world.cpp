#include <spdlog/spdlog.h>

#include "game_world.h"

#if defined(_WIN32) || defined(_WIN64)
constexpr auto SERVER_DLL_FILENAME = "server.dll";
#else
constexpr auto SERVER_DLL_FILENAME = "libserver.so";
#endif


int main(const int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::info);

    std::string server = SERVER_DLL_FILENAME;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.starts_with("--server=")) {
            server = arg.substr(strlen("--server="));
        }
    }

    const auto world = new GameWorld();

    utils::SetGameWorld(world);

    world->Init(server);
    world->Run();

    delete world;

    return 0;
}
