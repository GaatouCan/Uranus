#include "GameWorld.h"

int main(int argc, char *argv[]) {
    const auto world = new UGameWorld();

    world->Init("server.dll");
    world->Run();

    delete world;

    return 0;
}