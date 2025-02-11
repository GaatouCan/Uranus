#include "../../include/impl/package_codec.h"
#include "../../include/impl/package.h"
#include "../../include/connection.h"


#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif


PackageCodec::PackageCodec(Connection *conn)
    : TPackageCodec(conn) {
}

awaitable<void> PackageCodec::EncodeT(Package *pkg) {
    Package::Header header{};

    header.magic = htonl(pkg->mHeader.magic);
    header.version = htonl(pkg->mHeader.version);
    header.method = static_cast<CodecMethod>(htons(static_cast<uint16_t>(pkg->mHeader.method)));

    header.id = htonl(pkg->mHeader.id);
    header.length = htonll(pkg->mHeader.length);

    if (const auto len = co_await async_write(mConn->GetSocket(), asio::buffer(&header, Package::PACKAGE_HEADER_SIZE)); len == 0) {
        spdlog::warn("{} - Write package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->mHeader.length == 0)
        co_return;

    co_await async_write(mConn->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}

awaitable<void> PackageCodec::DecodeT(Package *pkg) {
    if (const auto len = co_await async_read(mConn->GetSocket(), asio::buffer(&pkg->mHeader, Package::PACKAGE_HEADER_SIZE)); len == 0) {
        spdlog::warn("{} - Read package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    pkg->mHeader.magic = ntohl(pkg->mHeader.magic);
    pkg->mHeader.version = ntohl(pkg->mHeader.version);
    pkg->mHeader.method = static_cast<CodecMethod>(ntohs(static_cast<uint16_t>(pkg->mHeader.method)));

    pkg->mHeader.id = ntohl(pkg->mHeader.id);
    pkg->mHeader.length = ntohll(pkg->mHeader.length);

    if (pkg->mHeader.length == 0)
        co_return;

    pkg->mData.Resize(pkg->mHeader.length);
    co_await async_read(mConn->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}
