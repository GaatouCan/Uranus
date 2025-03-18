#include "../include/scene/special_scene.h"

UGatherObject::UGatherObject(uint32_t id, uint32_t num)
    : id_(id),
      num_(num) {
}

UGatherObject::~UGatherObject() {
}

uint32_t UGatherObject::getGatherID() const {
    return id_;
}

uint32_t UGatherObject::getNum() const {
    return num_;
}

ISpecialScene::ISpecialScene(USceneManager *owner, int32_t id)
    : IBaseScene(owner, id) {
}

ISpecialScene::~ISpecialScene() {
    for (const auto &obj: objectList_)
        delete obj;
}

void ISpecialScene::onGatherObject(const std::shared_ptr<IBasePlayer> &plr, UGatherObject *obj) {
}
