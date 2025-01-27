#include "../../include/impl/PackageCodecImpl.h"
#include "../../include/impl/Package.h"
#include "../../include/Connection.h"

UPackageCodecImpl::UPackageCodecImpl(UConnection *conn)
    : TPackageCodec(conn) {
}

awaitable<void> UPackageCodecImpl::EncodeT(FPackage *pkg) {
    if (const auto len = co_await async_write(mConn->GetSocket(), asio::buffer(&pkg->header, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Write package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->header.length == 0)
        co_return;

    co_await async_write(mConn->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}

awaitable<void> UPackageCodecImpl::DecodeT(FPackage *pkg) {
    if (const auto len = co_await async_read(mConn->GetSocket(), asio::buffer(&pkg->header, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Read package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->header.length == 0)
        co_return;

    pkg->data.Resize(pkg->header.length);
    co_await async_read(mConn->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}
