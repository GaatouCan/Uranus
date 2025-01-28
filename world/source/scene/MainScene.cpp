#include "../../include/scene/MainScene.h"
#include "../../include/PackagePool.h"

UMainScene::UMainScene(USceneManager *owner, const uint32_t id)
    : IAbstractScene(owner, id),
      mPackagePool(new UPackagePool()) {
}

UMainScene::~UMainScene() {
    delete mPackagePool;
}

void UMainScene::SetThreadID(const AThreadID tid) {
    mThreadID = tid;
}

AThreadID UMainScene::GetThreadID() const {
    return mThreadID;
}

asio::io_context & UMainScene::GetIOContext() {
    return mIOContext;
}

UPackagePool * UMainScene::GetPackagePool() const {
    return mPackagePool;
}
