#include <spdlog/spdlog.h>

#include "game_world.h"


int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::info);

    const auto world = new GameWorld();

    world->Init("server.dll");
    world->Run();

    delete world;

    return 0;
}
