#pragma once

#include "common.h"

#include <memory>


class ProtocolRoute;
class IPackage;
class Connection;
class IBasePlayer;

using ProtoFunctor = void(*)(const std::shared_ptr<IBasePlayer> &, IPackage *);
// using ACrossFunctor = awaitable<void>(*)(IPackage *);

class BASE_API IProtocolHandler {

    ProtocolRoute *owner_;

public:
    IProtocolHandler() = delete;

    explicit IProtocolHandler(ProtocolRoute *route);
    virtual ~IProtocolHandler() = default;

    [[nodiscard]] ProtocolRoute *GetOwner() const;
    [[nodiscard]] class GameWorld *GetWorld() const;

    virtual void Invoke(const ProtoFunctor&, const std::shared_ptr<Connection>&, IPackage *) = 0;
    // virtual awaitable<void> InvokeCross(const ACrossFunctor&, IPackage *) = 0;
};
