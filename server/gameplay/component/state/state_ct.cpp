//
// Created by admin on 25-2-21.
//

#include "state_ct.h"
#include "../../../player/component_module.h"
#include "../../../player/cache_node.h"
#include "../../../player/player.h"

#include "utils.h"
#include <system/database/serializer.h>
#include <system/database/deserializer.h>

StateCT::StateCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {

}

StateCT::~StateCT() {
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

void StateCT::Serialize(Serializer* s) {
    WRITE_TABLE(s, State, mState)
}

void StateCT::Deserialize(Deserializer* ds) {
    READ_TABLE(ds, State, mState)
}
