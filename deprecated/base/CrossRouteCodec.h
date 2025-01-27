#pragma once

#include "common.h"
#include "Package.h"

#include <spdlog/spdlog.h>


class ICrossRouteCodec {

protected:

    ATcpSocket &mSocket;

public:
    ICrossRouteCodec() = delete;

    explicit ICrossRouteCodec(ATcpSocket &socket) : mSocket(socket) {}
    virtual ~ICrossRouteCodec() = default;

    virtual awaitable<void> Encode(IPackage *pkg) = 0;
    virtual awaitable<void> Decode(IPackage *pkg) = 0;
};

template<PACKAGE_TYPE T>
class TCrossRouteCodec : public ICrossRouteCodec {

public:
    explicit TCrossRouteCodec(ATcpSocket &socket) : ICrossRouteCodec(socket) {}

    awaitable<void> Encode(IPackage *pkg) override {
        try {
            co_await EncodeT(dynamic_cast<T *>(pkg));
        } catch (std::bad_cast &e) {
            pkg->Invalid();
            spdlog::error("{} - {}", __FUNCTION__, e.what());
        }
    }
    awaitable<void> Decode(IPackage *pkg) override {
        try {
            co_await DecodeT(dynamic_cast<T *>(pkg));
        } catch (std::bad_cast &e) {
            pkg->Invalid();
            spdlog::error("{} - {}", __FUNCTION__, e.what());
        } catch (std::system_error &e) {
            pkg->Invalid();
            spdlog::warn("{} - {}", __FUNCTION__, e.what());
        }
    }

    virtual awaitable<void> EncodeT(T *pkg) = 0;
    virtual awaitable<void> DecodeT(T *pkg) = 0;
};
