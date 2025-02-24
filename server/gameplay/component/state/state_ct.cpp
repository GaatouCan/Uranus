//
// Created by admin on 25-2-21.
//

#include "state_ct.h"
#include "../../../player/component_module.h"
#include "../../../player/cache_node.h"

#include "utils.h"
#include "../../../player/player.h"

StateCT::StateCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {

    COMPONENT_TABLE(StateCT, State)
}

StateCT::~StateCT() {
}

ISerializer * StateCT::Serialize_State(bool &bExpired) const {
    WRITE_TABLE(State, mState)
}

void StateCT::Deserialize_State(Deserializer &ds) {
    READ_TABLE(ds, mState)
}

void StateCT::OnLogin() {
    if (mState.pid == 0)
        mState.pid = GetOwner()->GetFullID();
}

int32_t StateCT::GetLevel() const {
    return mState.level;
}

int64_t StateCT::GetExp() const {
    return mState.experience;
}

void StateCT::SyncCache(CacheNode* node) {
    node->level = mState.level;
}
