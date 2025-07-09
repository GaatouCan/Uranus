#include "game_world.h"
#include <packet.h>

UGameWorld::UGameWorld(UContext *context)
    : Super(context),
      route_(this) {
}

UGameWorld::~UGameWorld() {
}

void UGameWorld::Initial(const std::shared_ptr<IPackage> &pkg) {
    Super::Initial(pkg);
    for (const auto &type : ordered_) {
        if (const auto iter = managerMap_.find(type); iter != managerMap_.end()) {
            iter->second->Initial();
        }
    }
}

void UGameWorld::Start() {
    Super::Start();
    for (const auto &type : ordered_) {
        if (const auto iter = managerMap_.find(type); iter != managerMap_.end()) {
            iter->second->BeginPlay();
        }
    }
}

void UGameWorld::Stop() {
    Super::Stop();
    for (const auto &type : ordered_) {
        if (const auto iter = managerMap_.find(type); iter != managerMap_.end()) {
            iter->second->EndPlay();
        }
    }
}

void UGameWorld::OnPackage(const std::shared_ptr<IPackage> &pkg) {
    const auto pkt = std::dynamic_pointer_cast<FPacket>(pkg);
    if (pkt == nullptr)
        return;

    route_.OnReceivePacket(pkt);
}

extern "C" SERVICE_API IService *NewService(UContext *context) {
    return new UGameWorld(context);
}

extern "C" SERVICE_API void DestroyService(IService *service) {
    delete service;
}
