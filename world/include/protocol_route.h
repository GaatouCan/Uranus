#pragma once

#include "protocol_handler.h"

#include <concepts>
#include <unordered_map>


class Connection;

class BASE_API ProtocolRoute final {

    friend class GameWorld;

    explicit ProtocolRoute(GameWorld* world);
    ~ProtocolRoute();

public:
    ProtocolRoute() = delete;

    DISABLE_COPY_MOVE(ProtocolRoute)

    void Init();

    [[nodiscard]] GameWorld* GetWorld() const;

    void RegisterProtocol(uint32_t type, const ProtoFunctor &func);
    // void RegisterCrossProtocol(uint32_t type, const ACrossFunctor &func);

    [[nodiscard]] ProtoFunctor FindProto(uint32_t proto) const;
    // [[nodiscard]] ACrossFunctor FindCross(uint32_t proto) const;

    template<typename T>
    requires std::derived_from<T, IProtocolHandler>
    void SetHandler() {
        if (mHandler != nullptr)
            mHandler.reset();

        mHandler = std::make_unique<T>(this);
    }

    void OnReadPackage(const std::shared_ptr<Connection> &conn, IPackage *pkg) const;
    // awaitable<void> OnCrossPackage(IPackage *pkg) const;

    void AbortHandler() const;
private:
    GameWorld *mWorld;

    std::unordered_map<uint32_t, ProtoFunctor> mProtoMap;
    // std::unordered_map<uint32_t, ACrossFunctor> mCrossMap;

    std::unique_ptr<IProtocolHandler> mHandler;
};

#define REGISTER_PROTOCOL(proto) \
    route->RegisterProtocol(static_cast<uint32_t>(ProtoType::proto), &proto);

#define REGISTER_FROM_CROSS(proto) \
    // route->RegisterCrossProtocol(static_cast<int32_t>(EProtoType::proto), &proto);
