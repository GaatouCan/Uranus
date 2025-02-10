#include "../../include/scene/MainScene.h"
#include "../../include/PackagePool.h"

UMainScene::UMainScene(USceneManager *owner, const int32_t id)
    : IAbstractScene(owner, id),
      pool_(new UPackagePool()) {
}

UMainScene::~UMainScene() {
    delete pool_;
}

void UMainScene::SetThreadID(const AThreadID tid) {
    tid_ = tid;
}

AThreadID UMainScene::GetThreadID() const {
    return tid_;
}

asio::io_context & UMainScene::GetIOContext() {
    return ctx_;
}

UPackagePool * UMainScene::GetPackagePool() const {
    return pool_;
}
