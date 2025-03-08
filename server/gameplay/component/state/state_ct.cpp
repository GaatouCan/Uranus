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

UStateCT::UStateCT(UComponentModule *module)
    : IPlayerComponent(module) {

}

UStateCT::~UStateCT() {
}

void UStateCT::onLogin() {
    if (state_.pid == 0)
        state_.pid = getOwner()->getFullID();
}

int32_t UStateCT::getLevel() const {
    return state_.level;
}

int64_t UStateCT::getExp() const {
    return state_.experience;
}

void UStateCT::syncCache(FCacheNode* node) {
    node->level = state_.level;
}

void UStateCT::serialize(const std::shared_ptr<USerializer> &s) {
    WRITE_TABLE(s, State, state_)
}

void UStateCT::deserialize(UDeserializer &ds) {
    READ_TABLE(&ds, State, state_)
}
