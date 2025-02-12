//
// Created by admin on 25-2-12.
//

#include "appearance_ct.h"
#include "../../../common/proto.def.h"

AppearanceCT::AppearanceCT(IComponentContext *ctx)
    : IPlayerComponent(ctx) {
}

AppearanceCT::~AppearanceCT() {
}

void protocol::AppearanceRequest(const std::shared_ptr<IBasePlayer> &plr, IPackage *pkg) {
    // TODO
}

