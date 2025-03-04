#include "CrossRouteCodecImpl.h"

UCrossRouteCodecImpl::UCrossRouteCodecImpl(ATcpSocket &socket)
    : TCrossRouteCodec(socket) {
}

awaitable<void> UCrossRouteCodecImpl::EncodeT(FPackage *pkg) {
    if (const auto len = co_await async_write(mSocket, asio::buffer(&pkg->header, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Write package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->header.length == 0)
        co_return;

    co_await async_write(mSocket, asio::buffer(pkg->RawByteArray().GetRawRef()));
}

awaitable<void> UCrossRouteCodecImpl::DecodeT(FPackage *pkg) {
    if (const auto len = co_await async_read(mSocket, asio::buffer(&pkg->header, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Read package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->header.length == 0)
        co_return;

    pkg->data.Resize(pkg->header.length);
    co_await async_read(mSocket, asio::buffer(pkg->RawByteArray().GetRawRef()));
}
