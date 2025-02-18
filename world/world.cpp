#include <spdlog/spdlog.h>

#include "game_world.h"


int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::info);

    const auto world = new GameWorld();

#if defined(_WIN32) || defined(_WIN64)
    world->Init("server.dll");
#else
    world->Init("libserver.so");
#endif

    world->Run();

    delete world;

    return 0;
}
