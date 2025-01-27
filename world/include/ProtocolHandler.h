#pragma once

#include "common.h"

#include <memory>


class UProtocolRoute;
class IPackage;
class UConnection;
class IBasePlayer;

using AProtoFunctor = void(*)(const std::shared_ptr<IBasePlayer> &, IPackage *);
// using ACrossFunctor = awaitable<void>(*)(IPackage *);

class BASE_API IProtocolHandler {

    UProtocolRoute *mOwner;

public:
    IProtocolHandler() = delete;

    explicit IProtocolHandler(UProtocolRoute *route);
    virtual ~IProtocolHandler() = default;

    [[nodiscard]] UProtocolRoute *GetOwner() const;
    [[nodiscard]] class UGameWorld *GetWorld() const;

    virtual void Invoke(const AProtoFunctor&, const std::shared_ptr<UConnection>&, IPackage *) = 0;
    // virtual awaitable<void> InvokeCross(const ACrossFunctor&, IPackage *) = 0;
};
