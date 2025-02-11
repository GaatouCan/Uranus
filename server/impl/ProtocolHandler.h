#pragma once

#include <ProtocolHandler.h>


class ProtocolHandler final : public IProtocolHandler {
public:
    explicit ProtocolHandler(ProtocolRoute *route);
    void Invoke(const ProtoFunctor &, const std::shared_ptr<Connection> &, IPackage *) override;

    // awaitable<void> InvokeCross(const ACrossFunctor &, IPackage *) override;
};
