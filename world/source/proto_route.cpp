#include "../include/proto_route.h"
#include "../include/package.h"

#include <spdlog/spdlog.h>

UProtoRoute::UProtoRoute(UGameWorld *world)
    : world_(world) {
}

UProtoRoute::~UProtoRoute() {

}

void UProtoRoute::init() {
}

UGameWorld *UProtoRoute::getWorld() const {
    return world_;
}


void UProtoRoute::registerProtocol(const uint32_t type, const AProtoFunctor &func) {
    protoMap_[type] = func;
}

// void UProtocolRoute::RegisterCrossProtocol(const uint32_t type, const ACrossFunctor &func) {
//     mCrossMap[type] = func;
// }

// ACrossFunctor UProtocolRoute::FindCross(const uint32_t proto) const {
//     if (const auto it = mCrossMap.find(proto); it != mCrossMap.end()) {
//         return it->second;
//     }
//     return nullptr;
// }

AProtoFunctor UProtoRoute::find(const uint32_t proto) const {
    if (const auto it = protoMap_.find(proto); it != protoMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void UProtoRoute::onReadPackage(const std::shared_ptr<UConnection> &conn, IPackage *pkg) const {
    // if (mHandler == nullptr) {
    //     spdlog::critical("{} - handler not set.", __FUNCTION__);
    //     return;
    // }

    if (!pkg->available()) {
        spdlog::warn("{} - Package unavailable", __FUNCTION__);
        return;
    }

    if (const auto func = find(pkg->getPackageID()); func)
        handler_->invoke(func, conn, pkg);
    else
        spdlog::warn("{} - Package[{}] Protocol functor unavailable.", __FUNCTION__, pkg->getPackageID());
}

void UProtoRoute::abort() const {
    assert(handler_ != nullptr);
}

// awaitable<void> UProtocolRoute::OnCrossPackage(IPackage *pkg) const {
//     if (mHandler == nullptr) {
//         spdlog::critical("{} - handler not set.", __FUNCTION__);
//         co_return;
//     }
//
//     if (!pkg->IsAvailable()) {
//         spdlog::warn("{} - Package unavailable", __FUNCTION__);
//         co_return;
//     }
//
//     if (const auto func = FindCross(pkg->GetPackageID()); func) {
//         co_await mHandler->InvokeCross(func, pkg);
//     } else {
//         spdlog::warn("{} - Package[{}] Protocol functor unavailable.", __FUNCTION__, pkg->GetPackageID());
//     }
// }
