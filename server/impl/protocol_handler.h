#pragma once

#include <protocol_handler.h>


class ProtocolHandler final : public IProtocolHandler {
public:
    explicit ProtocolHandler(UProtocolRoute *route);
    void Invoke(const ProtoFunctor &, const std::shared_ptr<UConnection> &, IPackage *) override;

    // awaitable<void> InvokeCross(const ACrossFunctor &, IPackage *) override;
};
