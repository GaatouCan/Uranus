#pragma once

#include "Package.h"

#include <asio/awaitable.hpp>
#include <spdlog/spdlog.h>

using asio::awaitable;

class IPackageCodec {

protected:
    class UConnection* mConn;

public:
    IPackageCodec() = delete;
    explicit IPackageCodec(UConnection *conn) : mConn(conn) {}

    virtual ~IPackageCodec() = default;

    virtual awaitable<void> Encode(IPackage *pkg) = 0;
    virtual awaitable<void> Decode(IPackage *pkg) = 0;
};


template<PACKAGE_TYPE T>
class BASE_API TPackageCodec : public IPackageCodec {
public:
    explicit TPackageCodec(UConnection *conn) : IPackageCodec(conn) {}
    ~TPackageCodec() override = default;

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