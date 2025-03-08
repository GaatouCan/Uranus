#pragma once

#include "common.h"

#include <memory>


class UProtoRoute;
class IPackage;
class UConnection;
class IBasePlayer;

using AProtoFunctor = void(*)(const std::shared_ptr<IBasePlayer> &, IPackage *);
// using ACrossFunctor = awaitable<void>(*)(IPackage *);

class BASE_API IProtoHandler {

    UProtoRoute *owner_;

public:
    IProtoHandler() = delete;

    explicit IProtoHandler(UProtoRoute *route);
    virtual ~IProtoHandler() = default;

    [[nodiscard]] UProtoRoute *getOwner() const;
    [[nodiscard]] class UGameWorld *getWorld() const;

    virtual void invoke(const AProtoFunctor&, const std::shared_ptr<UConnection>&, IPackage *) = 0;
    // virtual awaitable<void> InvokeCross(const ACrossFunctor&, IPackage *) = 0;
};
