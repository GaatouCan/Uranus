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

StateCT::StateCT(ComponentModule *module)
    : IPlayerComponent(module) {

}

StateCT::~StateCT() {
}

void StateCT::OnLogin() {
    if (state_.pid == 0)
        state_.pid = GetOwner()->GetFullID();
}

int32_t StateCT::GetLevel() const {
    return state_.level;
}

int64_t StateCT::GetExp() const {
    return state_.experience;
}

void StateCT::SyncCache(CacheNode* node) {
    node->level = state_.level;
}

void StateCT::Serialize(const std::shared_ptr<USerializer> &s) {
    WRITE_TABLE(s, State, state_)
}

void StateCT::Deserialize(UDeserializer &ds) {
    READ_TABLE(&ds, State, state_)
}
