#include "../../include/scene/main_scene.h"
#include "../../include/package_pool.h"

UMainScene::UMainScene(USceneManager *owner, const int32_t id)
    : IBaseScene(owner, id),
      pool_(new PackagePool()) {
}

UMainScene::~UMainScene() {
    delete pool_;
}

void UMainScene::SetThreadID(const AThreadID tid) {
    thread_ = tid;
}

AThreadID UMainScene::GetThreadID() const {
    return thread_;
}

asio::io_context & UMainScene::GetIOContext() {
    return ctx_;
}

PackagePool * UMainScene::GetPackagePool() const {
    return pool_;
}
