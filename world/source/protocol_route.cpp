#include "../include/protocol_route.h"
#include "../include/package.h"

#include <spdlog/spdlog.h>

ProtocolRoute::ProtocolRoute(GameWorld *world)
    : mWorld(world) {
}

ProtocolRoute::~ProtocolRoute() {

}

void ProtocolRoute::Init() {
}

GameWorld *ProtocolRoute::GetWorld() const {
    return mWorld;
}


void ProtocolRoute::RegisterProtocol(const uint32_t type, const ProtoFunctor &func) {
    mProtoMap[type] = func;
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
    if (const auto it = mProtoMap.find(proto); it != mProtoMap.end()) {
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
        mHandler->Invoke(func, conn, pkg);
    else
        spdlog::warn("{} - Package[{}] Protocol functor unavailable.", __FUNCTION__, pkg->GetPackageID());
}

void ProtocolRoute::AbortHandler() const {
    assert(mHandler != nullptr);
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
