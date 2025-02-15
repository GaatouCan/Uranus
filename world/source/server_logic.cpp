#include "../include/server_logic.h"
#include "../include/connection.h"
#include "../include/impl/package_codec.h"

IServerLogic::IServerLogic(GameWorld *world)
    : mWorld(world) {
}

IServerLogic::~IServerLogic() {
}

GameWorld *IServerLogic::GetWorld() const {
    return mWorld;
}

void IServerLogic::SetConnectionCodec(const std::shared_ptr<class Connection>& conn) {
    conn->SetPackageCodec<PackageCodec>();
}
