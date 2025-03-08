#include "../../include/scene/main_scene.h"
#include "../../include/package_pool.h"

UMainScene::UMainScene(USceneManager *owner, const int32_t id)
    : IBaseScene(owner, id),
      pool_(new UPackagePool()) {
}

UMainScene::~UMainScene() {
    delete pool_;
}

void UMainScene::setThreadID(const AThreadID tid) {
    thread_ = tid;
}

AThreadID UMainScene::getThreadID() const {
    return thread_;
}

asio::io_context & UMainScene::getIOContext() {
    return context_;
}

UPackagePool * UMainScene::getPackagePool() const {
    return pool_;
}
