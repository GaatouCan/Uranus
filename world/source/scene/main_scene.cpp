#include "../../include/scene/main_scene.h"
#include "../../include/package_pool.h"

MainScene::MainScene(SceneManager *owner, const int32_t id)
    : IBaseScene(owner, id),
      mPool(new PackagePool()) {
}

MainScene::~MainScene() {
    delete mPool;
}

void MainScene::SetThreadID(const ThreadID tid) {
    mThreadID = tid;
}

ThreadID MainScene::GetThreadID() const {
    return mThreadID;
}

asio::io_context & MainScene::GetIOContext() {
    return mContext;
}

PackagePool * MainScene::GetPackagePool() const {
    return mPool;
}
