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

void IServerLogic::setPackageCodec(const std::shared_ptr<UConnection> &conn) {
    conn->setPackageCodec<UPackageCodec>();
}

IRecycler *IServerLogic::createPackagePool() {
    const auto &cfg = getWorld()->getServerConfig();

    const auto packageMagic = cfg["package"]["magic"].as<uint32_t>();
    const auto packageVersion = cfg["package"]["version"].as<uint32_t>();

    auto codecMethod = ECodecMethod::UNAVAILABLE;
    const auto str = cfg["package"]["method"].as<std::string>();
    if (str == "LineBased")
        codecMethod = ECodecMethod::BASE_LINE;
    if (str == "Protobuf")
        codecMethod = ECodecMethod::PROTOBUF;

    const auto defaultCapacity = cfg["package"]["pool"]["default_capacity"].as<size_t>();
    const auto expanseScale = cfg["package"]["pool"]["expanse_scale"].as<float>();

    const auto pool = new TRecycler<FPackage>();

    pool->setCreateCallback([packageMagic, packageVersion, codecMethod](FPackage *pkg) {
        pkg->setMagic(packageMagic);
        pkg->setVersion(packageVersion);
        pkg->setMethod(codecMethod);
    });

    pool->setCapacity(defaultCapacity).setExpanseScale(expanseScale);

    return pool;
}
