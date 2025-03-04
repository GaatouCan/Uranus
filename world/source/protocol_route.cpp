#include "../include/protocol_route.h"
#include "../include/package.h"

#include <spdlog/spdlog.h>

ProtocolRoute::ProtocolRoute(GameWorld *world)
    : world_(world) {
}

ProtocolRoute::~ProtocolRoute() {

}

void ProtocolRoute::Init() {
}

GameWorld *ProtocolRoute::GetWorld() const {
    return world_;
}


void ProtocolRoute::RegisterProtocol(const uint32_t type, const ProtoFunctor &func) {
    proto_map_[type] = func;
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

ProtoFunctor ProtocolRoute::FindProto(const uint32_t proto) const {
    if (const auto it = proto_map_.find(proto); it != proto_map_.end()) {
        return it->second;
    }
    return nullptr;
}

void ProtocolRoute::OnReadPackage(const std::shared_ptr<Connection> &conn, IPackage *pkg) const {
    // if (mHandler == nullptr) {
    //     spdlog::critical("{} - handler not set.", __FUNCTION__);
    //     return;
    // }

    if (!pkg->IsAvailable()) {
        spdlog::warn("{} - Package unavailable", __FUNCTION__);
        return;
    }

    if (const auto func = FindProto(pkg->GetPackageID()); func)
        handler_->Invoke(func, conn, pkg);
    else
        spdlog::warn("{} - Package[{}] Protocol functor unavailable.", __FUNCTION__, pkg->GetPackageID());
}

void ProtocolRoute::AbortHandler() const {
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
