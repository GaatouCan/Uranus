#pragma once

#include "package.h"

#include <asio/awaitable.hpp>
#include <spdlog/spdlog.h>

using asio::awaitable;

class IPackageCodec {

protected:
    std::weak_ptr<class UConnection> conn_;

public:
    IPackageCodec() = delete;
    explicit IPackageCodec(const std::weak_ptr<UConnection> &conn) : conn_(conn) {}

    virtual ~IPackageCodec() = default;

    virtual awaitable<void> encode(IPackage *pkg) = 0;
    virtual awaitable<void> decode(IPackage *pkg) = 0;
};


template<CPackageType T>
class BASE_API TPackageCodec : public IPackageCodec {
public:
    explicit TPackageCodec(const std::weak_ptr<UConnection> &conn) : IPackageCodec(conn) {}
    ~TPackageCodec() override = default;

    awaitable<void> encode(IPackage *pkg) override {
        try {
            co_await encodeT(dynamic_cast<T *>(pkg));
        } catch (std::bad_cast &e) {
            pkg->invalid();
            spdlog::error("{} - {}", __FUNCTION__, e.what());
        }
    }
    awaitable<void> decode(IPackage *pkg) override {
        try {
            co_await decodeT(dynamic_cast<T *>(pkg));
        } catch (std::bad_cast &e) {
            pkg->invalid();
            spdlog::error("{} - {}", __FUNCTION__, e.what());
        } catch (std::system_error &e) {
            pkg->invalid();
            spdlog::warn("{} - {}", __FUNCTION__, e.what());
        }
    }

    virtual awaitable<void> encodeT(T *pkg) = 0;
    virtual awaitable<void> decodeT(T *pkg) = 0;
};