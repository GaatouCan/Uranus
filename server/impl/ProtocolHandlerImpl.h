#pragma once

#include "ProtocolHandler.h"

class UProtocolHandlerImpl final : public IProtocolHandler {
public:
    explicit UProtocolHandlerImpl(ProtocolRoute *route);
    void Invoke(const ProtoFunctor &, const std::shared_ptr<Connection> &, IPackage *) override;

    // awaitable<void> InvokeCross(const ACrossFunctor &, IPackage *) override;
};
