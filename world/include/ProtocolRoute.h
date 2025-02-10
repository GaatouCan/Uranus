#pragma once

#include "ProtocolHandler.h"

#include <concepts>
#include <unordered_map>


class UConnection;

class BASE_API UProtocolRoute final {

    friend class UGameWorld;

    explicit UProtocolRoute(UGameWorld* world);
    ~UProtocolRoute();

public:
    UProtocolRoute() = delete;

    DISABLE_COPY_MOVE(UProtocolRoute)

    void Init();

    [[nodiscard]] UGameWorld* GetWorld() const;

    void RegisterProtocol(uint32_t type, const AProtoFunctor &func);
    // void RegisterCrossProtocol(uint32_t type, const ACrossFunctor &func);

    [[nodiscard]] AProtoFunctor FindProto(uint32_t proto) const;
    // [[nodiscard]] ACrossFunctor FindCross(uint32_t proto) const;

    template<typename T>
    requires std::derived_from<T, IProtocolHandler>
    void SetHandler() {
        if (handler_ != nullptr)
            handler_.reset();

        handler_ = std::make_unique<T>(this);
    }

    void OnReadPackage(const std::shared_ptr<UConnection> &conn, IPackage *pkg) const;
    // awaitable<void> OnCrossPackage(IPackage *pkg) const;

    void AbortHandler() const;
private:
    UGameWorld *world_;

    std::unordered_map<uint32_t, AProtoFunctor> protoMap_;
    // std::unordered_map<uint32_t, ACrossFunctor> mCrossMap;

    std::unique_ptr<IProtocolHandler> handler_;
};

#define REGISTER_PROTOCOL(proto) \
    route->RegisterProtocol(static_cast<int32_t>(EProtoType::proto), &proto);

#define REGISTER_FROM_CROSS(proto) \
    // route->RegisterCrossProtocol(static_cast<int32_t>(EProtoType::proto), &proto);
