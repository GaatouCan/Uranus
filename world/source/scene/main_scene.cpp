#include "../../include/scene/main_scene.h"
#include "../../include/package_pool.h"

MainScene::MainScene(SceneManager *owner, const int32_t id)
    : IBaseScene(owner, id),
      pool_(new PackagePool()) {
}

MainScene::~MainScene() {
    delete pool_;
}

void MainScene::SetThreadID(const ThreadID tid) {
    thread_ = tid;
}

ThreadID MainScene::GetThreadID() const {
    return thread_;
}

asio::io_context & MainScene::GetIOContext() {
    return ctx_;
}

PackagePool * MainScene::GetPackagePool() const {
    return pool_;
}
