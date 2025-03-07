#include "../include/server_logic.h"
#include "../include/connection.h"
#include "../include/impl/package_codec.h"

IServerLogic::IServerLogic(UGameWorld *world)
    : world_(world) {
}

IServerLogic::~IServerLogic() {
}

UGameWorld *IServerLogic::GetWorld() const {
    return world_;
}

void IServerLogic::SetConnectionCodec(const std::shared_ptr<class UConnection>& conn) {
    conn->SetPackageCodec<UPackageCodec>();
}
