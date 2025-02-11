#pragma once

#include "../DefaultCodec.h"
#include "Package.h"

class DefaultCodec final : public TPackageCodec<Package> {
public:
    explicit DefaultCodec(Connection *conn);

    awaitable<void> EncodeT(Package *pkg) override;
    awaitable<void> DecodeT(Package *pkg) override;
};
