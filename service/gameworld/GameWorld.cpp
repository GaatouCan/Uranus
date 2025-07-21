#include "GameWorld.h"

#include <Config/Config.h>

UGameWorld::UGameWorld() {
    mRoute.SetUpGameWorld(this);
}

UGameWorld::~UGameWorld() {
}

bool UGameWorld::Initial(const std::shared_ptr<IPackageInterface> &pkg) {
    if (!Super::Initial(pkg))
        return false;

    if (const auto res = mConfig.LoadConfig(GetModule<UConfig>()); res != 0)
        return false;

    for (const auto &type : mOrdered) {
        if (const auto iter = mManagerMap.find(type); iter != mManagerMap.end()) {
            iter->second->Initial();
        }
    }

    return true;
}

bool UGameWorld::Start() {
    if (!Super::Start())
        return false;

    for (const auto &type : mOrdered) {
        if (const auto iter = mManagerMap.find(type); iter != mManagerMap.end()) {
            iter->second->BeginPlay();
        }
    }
    return true;
}

void UGameWorld::Stop() {
    Super::Stop();
    for (const auto &type : mOrdered) {
        if (const auto iter = mManagerMap.find(type); iter != mManagerMap.end()) {
            iter->second->EndPlay();
        }
    }
}

void UGameWorld::OnPackage(const std::shared_ptr<IPackageInterface> &pkg) {
    const auto pkt = std::dynamic_pointer_cast<FPacket>(pkg);
    if (pkt == nullptr)
        return;

    mRoute.OnReceivePacket(pkt);
}

extern "C" SERVICE_API IServiceBase *CreateInstance() {
    return new UGameWorld();
}

extern "C" SERVICE_API void DestroyInstance(IServiceBase *service) {
    delete dynamic_cast<UGameWorld *>(service);
}
