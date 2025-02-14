//
// Created by admin on 25-2-12.
//

#include "appearance_ct.h"

#include "utils.h"
#include "../../../common/proto.def.h"

#include "../../../player/component_module.h"

AppearanceCT::AppearanceCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {

    SERIALIZE_COMPONENT(AppearanceCT, Appearance)
}

AppearanceCT::~AppearanceCT() {
}

ISerializer * AppearanceCT::Serialize_Appearance(bool &bExpired) const {
    READ_PARAM(Appearance, mAppear)
}

void AppearanceCT::Deserialize_Appearance(Deserializer &ds) {
    WRITE_PARAM(ds, mAppear)
}

void protocol::AppearanceRequest(const std::shared_ptr<IBasePlayer> &plr, IPackage *pkg) {
    // TODO
}

