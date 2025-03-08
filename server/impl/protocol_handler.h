#pragma once

#include <proto_handler.h>


class ProtocolHandler final : public IProtoHandler {
public:
    explicit ProtocolHandler(UProtoRoute *route);
    void invoke(const AProtoFunctor &, const std::shared_ptr<UConnection> &, IPackage *) override;

    // awaitable<void> InvokeCross(const ACrossFunctor &, IPackage *) override;
};
