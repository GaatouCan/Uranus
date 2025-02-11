#pragma once

#include "../package_codec.h"
#include "package.h"

class DefaultCodec final : public TPackageCodec<Package> {
public:
    explicit DefaultCodec(Connection *conn);

    awaitable<void> EncodeT(Package *pkg) override;
    awaitable<void> DecodeT(Package *pkg) override;
};
