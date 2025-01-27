#pragma once

#include "../CrossRouteCodec.h"
#include "Package.h"


class UCrossRouteCodecImpl final : public TCrossRouteCodec<FPackage> {

public:
    explicit UCrossRouteCodecImpl(ATcpSocket &socket);

    awaitable<void> EncodeT(FPackage *pkg) override;
    awaitable<void> DecodeT(FPackage *pkg) override;
};

