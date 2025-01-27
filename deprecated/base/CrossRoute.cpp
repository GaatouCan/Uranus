#include "CrossRoute.h"

#include <spdlog/spdlog.h>


UCrossRoute::UCrossRoute()
    : mSocket(mIOContext){

}

UCrossRoute::~UCrossRoute() {
    Disconnect();
}

UCrossRoute & UCrossRoute::Instance() {
    static UCrossRoute instance;
    return instance;
}

UCrossRoute & UCrossRoute::Run() {
    asio::signal_set signals(mIOContext, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        Disconnect();
    });

    mIOContext.run();
    return *this;
}

UCrossRoute &UCrossRoute::ConnectToCross(const std::string &host, const std::string &port) {
    try {
        tcp::resolver resolver(mIOContext);
        const auto endpoint = resolver.resolve(host, port);
        asio::connect(mSocket, endpoint);

        if (mHandler)
            mHandler->OnConnect();

        co_spawn(mIOContext, [this]() mutable -> awaitable<void> {
            try {
                co_await ReadPackage();
            } catch (std::exception &e) {
                spdlog::error("UCrossRoute::connectToCross: {}", e.what());
            }
        }, detached);
    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
    }

    return *this;
}

UCrossRoute &UCrossRoute::Disconnect() {
    if (mSocket.is_open()) {
        mSocket.close();
    }

    return *this;
}

bool UCrossRoute::IsConnected() const {
    return mSocket.is_open();
}

void UCrossRoute::RegisterProtocol(const uint32_t proto) {
    mCrossProto.insert(proto);
}

bool UCrossRoute::IsCrossProtocol(const uint32_t proto) const {
    return IsConnected() ? mCrossProto.contains(proto) : false;
}

UCrossRoute &UCrossRoute::Send(IPackage *pkg) {
    const bool bEmpty = mOutput.IsEmpty();
    mOutput.PushBack(pkg);

    if (bEmpty)
        co_spawn(mSocket.get_executor(), WritePackage(), detached);

    return *this;
}

awaitable<void> UCrossRoute::WritePackage() {
    try {
        if (mCodec == nullptr) {
            spdlog::critical("{} - codec undefined", __FUNCTION__);
            Disconnect();
            co_return;
        }

        while (mSocket.is_open() && !mOutput.IsEmpty()) {
            const auto pkg = mOutput.PopFront();

            co_await mCodec->Encode(pkg);

            if (pkg->IsAvailable()) {
                if (mHandler != nullptr) {
                    co_await mHandler->OnWritePackage(pkg);
                }
                mPool.Recycle(pkg);
            } else {
                spdlog::warn("{} - Write Failed", __FUNCTION__);
                mPool.Recycle(pkg);
                Disconnect();
            }
        }
    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
        Disconnect();
    }
}

awaitable<void> UCrossRoute::ReadPackage() {
    try {
        if (mCodec == nullptr) {
            spdlog::error("{} - Codec is null", __FUNCTION__);
            Disconnect();
            co_return;
        }

        while (mSocket.is_open()) {
            const auto pkg = mPool.Acquire();
            co_await mCodec->Decode(pkg);

            if (pkg->IsAvailable()) {
                if (mHandler != nullptr) {
                    co_await mHandler->OnWritePackage(pkg);
                }
                mPool.Recycle(pkg);
            } else {
                spdlog::warn("{} - Write Failed", __FUNCTION__);
                mPool.Recycle(pkg);
                Disconnect();
            }
        }

    } catch (std::exception &e) {
        spdlog::error("{} - {}", __FUNCTION__, e.what());
        Disconnect();
    }
}
