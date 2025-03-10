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

void IServerLogic::setConnectionCodec(const std::shared_ptr<UConnection>& conn) {
    conn->setPackageCodec<UPackageCodec>();
}

void IServerLogic::setPackage() {
    const auto &cfg = getWorld()->getServerConfig();
    FPackage::LoadConfig(cfg);
}

IRecycler * IServerLogic::createPackagePool() {
    const auto &cfg = getWorld()->getServerConfig();
    const auto pool = new TRecycler<FPackage>();

    if (cfg["package"].IsNull() && cfg["package"]["pool"].IsNull())
        return pool;

    if (!cfg["package"]["pool"]["minimum_capacity"].IsNull())
        pool->setMinimumCapacity(cfg["package"]["pool"]["minimum_capacity"].as<size_t>());

    if (!cfg["package"]["pool"]["expanse_rate"].IsNull())
        pool->setExpanseRate(cfg["package"]["pool"]["expanse_rate"].as<float>());

    if (!cfg["package"]["pool"]["expanse_scale"].IsNull())
        pool->setExpanseScale(cfg["package"]["pool"]["expanse_scale"].as<float>());

    if (!cfg["package"]["pool"]["collect_rate"].IsNull())
        pool->setCollectRate(cfg["package"]["pool"]["collect_rate"].as<float>());

    if (!cfg["package"]["pool"]["collect_scale"].IsNull())
        pool->setCollectScale(cfg["package"]["pool"]["collect_scale"].as<float>());

    if (!cfg["package"]["pool"]["default_capacity"].IsNull())
        pool->init(cfg["package"]["pool"]["default_capacity"].as<size_t>());

    return pool;
}
