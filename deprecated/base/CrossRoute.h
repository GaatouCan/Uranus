#pragma once

#include "common.h"
#include "CrossRouteCodec.h"
#include "CrossRouteHandler.h"
#include "PackagePool.h"
#include "TSDeque.h"


class UCrossRoute final {

    asio::io_context mIOContext;
    ATcpSocket mSocket;

    UPackagePool mPool;
    TSDeque<IPackage *> mOutput;

    std::unique_ptr<ICrossRouteCodec> mCodec;
    std::unique_ptr<ICrossRouteHandler> mHandler;

    std::set<uint32_t> mCrossProto;

private:
    UCrossRoute();
    ~UCrossRoute();

public:
    DISABLE_COPY_MOVE(UCrossRoute)

    static UCrossRoute &Instance();

    UCrossRoute &Run();

    UCrossRoute &ConnectToCross(const std::string &host, const std::string &port);
    UCrossRoute &Disconnect();

    [[nodiscard]] bool IsConnected() const;

    void RegisterProtocol(uint32_t proto);
    [[nodiscard]] bool IsCrossProtocol(uint32_t proto) const;

    UCrossRoute &Send(IPackage *pkg);

    template<typename T>
    requires std::derived_from<T, ICrossRouteCodec>
    UCrossRoute &SetPackageCodec() {
        if (mCodec != nullptr)
            mCodec.reset();

        mCodec = std::make_unique<T>(mSocket);
        return *this;
    }

    template<typename T>
    requires std::derived_from<T, ICrossRouteHandler>
    UCrossRoute &SetHandler() {
        if (mHandler != nullptr)
            mHandler.reset();

        mHandler = std::make_unique<T>();
        return *this;
    }

private:
    awaitable<void> WritePackage();
    awaitable<void> ReadPackage();
};

#define REGISTER_TO_CROSS(proto) \
UCrossRoute::Instance().RegisterProtocol(static_cast<uint32_t>(EProtoType::proto));
