#include <spdlog/spdlog.h>

#include "game_world.h"

#if defined(_WIN32) || defined(_WIN64)
    constexpr auto SERVER_DLL_FILENAME = "server.dll";
#else
    constexpr auto SERVER_DLL_FILENAME = "libserver.so";
#endif


int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::info);

    const auto world = new GameWorld();

    world->Init(SERVER_DLL_FILENAME);
    world->Run();

    delete world;

    return 0;
}
