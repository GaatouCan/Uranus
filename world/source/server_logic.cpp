#include "../include/server_logic.h"

#include "../include/recycler.h"
#include "../include/game_world.h"
#include "../include/connection.h"
#include "../include/impl/package_codec.h"
#include "../include/impl/package.h"

IServerLogic::IServerLogic(UGameWorld *world)
    : world_(world) {
}

IServerLogic::~IServerLogic() {
}

UGameWorld *IServerLogic::getWorld() const {
    return world_;
}

void IServerLogic::setConnectionCodec(const std::shared_ptr<UConnection> &conn) {
    conn->setPackageCodec<UPackageCodec>();
}

void IServerLogic::setPackage() {
    const auto &cfg = getWorld()->getServerConfig();
    FPackage::LoadConfig(cfg);
}

IRecycler *IServerLogic::createPackagePool() {
    const auto &cfg = getWorld()->getServerConfig();

    const auto defaultCapacity = cfg["package"]["pool"]["default_capacity"].as<size_t>();
    // const auto minimumCapacity = cfg["package"]["pool"]["minimum_capacity"].as<size_t>();

    // const auto expanseRate = cfg["package"]["pool"]["expanse_rate"].as<float>();
    const auto expanseScale = cfg["package"]["pool"]["expanse_scale"].as<float>();

    // const auto collectRate = cfg["package"]["pool"]["collect_rate"].as<float>();
    // const auto collectScale = cfg["package"]["pool"]["collect_scale"].as<float>();

    const auto pool = new TRecycler<FPackage>();

    pool->setCapacity(defaultCapacity).setExpanseScale(expanseScale);
            // .setMinimumCapacity(minimumCapacity)
            // .setExpanseRate(expanseRate)
            // .setCollectRate(collectRate)
            // .setCollectScale(collectScale);

    return pool;
}
