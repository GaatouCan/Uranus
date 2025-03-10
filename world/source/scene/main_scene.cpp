#include "../../include/scene/main_scene.h"
#include "../../include/game_world.h"
#include "../../include/recycler.h"

UMainScene::UMainScene(USceneManager *owner, const int32_t id, IRecycler *pool)
    : IBaseScene(owner, id),
      pool_(pool) {
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

asio::io_context &UMainScene::getIOContext() {
    return context_;
}

IRecycler *UMainScene::getPackagePool() const {
    return pool_;
}
