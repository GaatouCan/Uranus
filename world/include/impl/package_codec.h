#pragma once

#include "../package_codec.h"
#include "package.h"

class PackageCodec final : public TPackageCodec<Package> {
public:
    explicit PackageCodec(Connection *conn);

    awaitable<void> EncodeT(Package *pkg) override;
    awaitable<void> DecodeT(Package *pkg) override;
};
