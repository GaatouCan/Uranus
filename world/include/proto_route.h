#pragma once

#include "proto_handler.h"

#include <concepts>
#include <absl/container/flat_hash_map.h>


class UConnection;

class BASE_API UProtoRoute final {

    friend class UGameWorld;

    explicit UProtoRoute(UGameWorld* world);
    ~UProtoRoute();

public:
    UProtoRoute() = delete;

    DISABLE_COPY_MOVE(UProtoRoute)

    void init();

    [[nodiscard]] UGameWorld* getWorld() const;

    void registerProtocol(uint32_t type, const AProtoFunctor &func);
    // void RegisterCrossProtocol(uint32_t type, const ACrossFunctor &func);

    [[nodiscard]] AProtoFunctor find(uint32_t proto) const;
    // [[nodiscard]] ACrossFunctor FindCross(uint32_t proto) const;

    template<typename T, typename... Args>
    requires std::derived_from<T, IProtoHandler>
    void setHandler(Args &&... args) {
        if (handler_ != nullptr)
            handler_.reset();

        handler_ = std::make_unique<T>(this, std::forward<Args>(args)...);
    }

    void onReadPackage(const std::shared_ptr<UConnection> &conn, IPackage *pkg) const;
    // awaitable<void> OnCrossPackage(IPackage *pkg) const;

    void abort() const;
private:
    UGameWorld *world_;

    absl::flat_hash_map<uint32_t, AProtoFunctor> protoMap_;
    // std::unordered_map<uint32_t, ACrossFunctor> mCrossMap;

    std::unique_ptr<IProtoHandler> handler_;
};

#define REGISTER_PROTOCOL(proto) \
    route->registerProtocol(static_cast<uint32_t>(EProtoType::proto), &proto);

#define REGISTER_FROM_CROSS(proto) \
    // route->RegisterCrossProtocol(static_cast<int32_t>(EProtoType::proto), &proto);
