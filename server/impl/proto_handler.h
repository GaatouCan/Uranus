#pragma once

#include <proto_handler.h>


class UProtoHandler final : public IProtoHandler {
public:
    explicit UProtoHandler(UProtoRoute *route);
    void invoke(const AProtoFunctor &, const std::shared_ptr<UConnection> &, IPackage *) override;

    // awaitable<void> InvokeCross(const ACrossFunctor &, IPackage *) override;
};
