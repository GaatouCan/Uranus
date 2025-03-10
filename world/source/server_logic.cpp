#include "../include/server_logic.h"

#include "../include/recycler.h"
#include "../include/game_world.h"
#include "../include/connection.h"
#include "../include/impl/package_codec.h"
#include "../include/impl/package.h"
#include "../include/scene/main_scene.h"

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

void IServerLogic::setPackagePool(UMainScene *scene) {
    scene->pool_ = new TRecycler<FPackage>();

    const auto &cfg = getWorld()->getServerConfig();

    if (cfg["package"].IsNull() && cfg["package"]["pool"].IsNull())
        return;

    if (!cfg["package"]["pool"]["minimum_capacity"].IsNull())
        scene->pool_->setMinimumCapacity(cfg["package"]["pool"]["minimum_capacity"].as<size_t>());

    if (!cfg["package"]["pool"]["expanse_rate"].IsNull())
        scene->pool_->setExpanseRate(cfg["package"]["pool"]["expanse_rate"].as<float>());

    if (!cfg["package"]["pool"]["expanse_scale"].IsNull())
        scene->pool_->setExpanseScale(cfg["package"]["pool"]["expanse_scale"].as<float>());

    if (!cfg["package"]["pool"]["collect_rate"].IsNull())
        scene->pool_->setCollectRate(cfg["package"]["pool"]["collect_rate"].as<float>());

    if (!cfg["package"]["pool"]["collect_scale"].IsNull())
        scene->pool_->setCollectScale(cfg["package"]["pool"]["collect_scale"].as<float>());

    if (!cfg["package"]["pool"]["default_capacity"].IsNull())
        scene->pool_->init(cfg["package"]["pool"]["default_capacity"].as<size_t>());
}
