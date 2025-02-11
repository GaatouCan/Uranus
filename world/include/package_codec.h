#pragma once

#include "package.h"

#include <asio/awaitable.hpp>
#include <spdlog/spdlog.h>

using asio::awaitable;

class IPackageCodec {

protected:
    class Connection* mConn;

public:
    IPackageCodec() = delete;
    explicit IPackageCodec(Connection *conn) : mConn(conn) {}

    virtual ~IPackageCodec() = default;

    virtual awaitable<void> Encode(IPackage *pkg) = 0;
    virtual awaitable<void> Decode(IPackage *pkg) = 0;
};


template<PackageType T>
class BASE_API TPackageCodec : public IPackageCodec {
public:
    explicit TPackageCodec(Connection *conn) : IPackageCodec(conn) {}
    ~TPackageCodec() override = default;

    awaitable<void> Encode(IPackage *pkg) override {
        try {
            co_await this->EncodeT(dynamic_cast<T *>(pkg));
        } catch (std::bad_cast &e) {
            pkg->Invalid();
            spdlog::error("{} - {}", __FUNCTION__, e.what());
        }
    }
    awaitable<void> Decode(IPackage *pkg) override {
        try {
            co_await this->DecodeT(dynamic_cast<T *>(pkg));
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