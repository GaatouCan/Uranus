#include "../include/ProtocolRoute.h"
#include "../include/Package.h"

#include <spdlog/spdlog.h>

UProtocolRoute::UProtocolRoute(UGameWorld *world)
    : mWorld(world) {
}

UProtocolRoute::~UProtocolRoute() {

}

void UProtocolRoute::Init() {
}

UGameWorld *UProtocolRoute::GetWorld() const {
    return mWorld;
}


void UProtocolRoute::RegisterProtocol(const uint32_t type, const AProtoFunctor &func) {
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

AProtoFunctor UProtocolRoute::FindProto(const uint32_t proto) const {
    if (const auto it = mProtoMap.find(proto); it != mProtoMap.end()) {
        return it->second;
    }
    return nullptr;
}

void UProtocolRoute::OnReadPackage(const std::shared_ptr<UConnection> &conn, IPackage *pkg) const {
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

void UProtocolRoute::AbortHandler() const {
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
