#pragma once

#include "../package_codec.h"
#include "package.h"

class UPackageCodec final : public TPackageCodec<FPackage> {
public:
    explicit UPackageCodec(const std::weak_ptr<UConnection> &conn);

    awaitable<void> EncodeT(FPackage *pkg) override;
    awaitable<void> DecodeT(FPackage *pkg) override;
};
