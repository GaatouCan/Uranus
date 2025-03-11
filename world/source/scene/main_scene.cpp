#include "../../include/scene/main_scene.h"
#include "../../include/server_logic.h"
#include "../../include/game_world.h"
#include "../../include/recycler.h"

UMainScene::UMainScene(USceneManager *owner, const int32_t id)
    : IBaseScene(owner, id),
      pool_(nullptr) {

    if (const auto server = getWorld()->getServerLogic(); server != nullptr) {
        pool_ = server->createPackagePool();
    }

    assert(pool_ != nullptr);
    pool_->init();
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
