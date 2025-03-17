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

    virtual awaitable<bool> encode(IPackage *pkg) = 0;
    virtual awaitable<bool> decode(IPackage *pkg) = 0;
};


template<CPackageType T>
class BASE_API TPackageCodec : public IPackageCodec {
public:
    explicit TPackageCodec(const std::weak_ptr<UConnection> &conn) : IPackageCodec(conn) {}
    ~TPackageCodec() override = default;

    awaitable<bool> encode(IPackage *pkg) override {
        try {
            const auto res = co_await encodeT(dynamic_cast<T *>(pkg));
            co_return res;
        } catch (std::bad_cast &e) {
            spdlog::error("{} - {}", __FUNCTION__, e.what());
            co_return false;
        }
    }

    awaitable<bool> decode(IPackage *pkg) override {
        try {
            const auto res = co_await decodeT(dynamic_cast<T *>(pkg));
            co_return res;
        } catch (std::bad_cast &e) {
            spdlog::error("{} - {}", __FUNCTION__, e.what());
            co_return false;
        } catch (std::system_error &e) {
            spdlog::warn("{} - {}", __FUNCTION__, e.what());
            co_return false;
        }
    }

    virtual awaitable<bool> encodeT(T *pkg) = 0;
    virtual awaitable<bool> decodeT(T *pkg) = 0;
};