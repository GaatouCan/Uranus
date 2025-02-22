//
// Created by admin on 25-2-21.
//

#include "state_ct.h"
#include "../../../player/component_module.h"

#include "utils.h"

StateCT::StateCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {
}

StateCT::~StateCT() {
}

ISerializer * StateCT::Serialize_State(bool &bExpired) const {
    WRITE_PARAM(State, mState)
}

void StateCT::Deserialize_State(Deserializer &ds) {
    READ_PARAM(ds, mState)
}
