#pragma once

#include "ProtocolHandler.h"

class UProtocolHandlerImpl final : public IProtocolHandler {
public:
    explicit UProtocolHandlerImpl(UProtocolRoute *route);
    void Invoke(const AProtoFunctor &, const std::shared_ptr<UConnection> &, IPackage *) override;

    // awaitable<void> InvokeCross(const ACrossFunctor &, IPackage *) override;
};
