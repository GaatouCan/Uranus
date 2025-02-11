#pragma once

#include "../PackageCodec.h"
#include "Package.h"

class PackageCodec final : public TPackageCodec<Package> {
public:
    explicit PackageCodec(Connection *conn);

    awaitable<void> EncodeT(Package *pkg) override;
    awaitable<void> DecodeT(Package *pkg) override;
};
