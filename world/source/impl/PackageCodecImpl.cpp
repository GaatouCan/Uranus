#include "../../include/impl/PackageCodecImpl.h"
#include "../../include/impl/Package.h"
#include "../../include/Connection.h"


#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
mModule = dlopen(path.c_str(), RTLD_LAZY);
#endif


UPackageCodecImpl::UPackageCodecImpl(UConnection *conn)
    : TPackageCodec(conn) {
}

awaitable<void> UPackageCodecImpl::EncodeT(FPackage *pkg) {
    FPackage::FHeader header{};

    header.magic = htonl(pkg->header.magic);
    header.version = htonl(pkg->header.version);
    header.method = static_cast<ECodecMethod>(htons(static_cast<uint16_t>(pkg->header.method)));

    header.id = htonl(pkg->header.id);
    header.length = htonll(pkg->header.length);

    if (const auto len = co_await async_write(mConn->GetSocket(), asio::buffer(&header, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Write package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->header.length == 0)
        co_return;

    co_await async_write(mConn->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}

awaitable<void> UPackageCodecImpl::DecodeT(FPackage *pkg) {
    FPackage::FHeader header{};
    memset(&header, 0, sizeof(header));

    if (const auto len = co_await async_read(mConn->GetSocket(), asio::buffer(&header, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Read package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    pkg->header.magic = ntohl(header.magic);
    pkg->header.version = ntohl(header.version);
    pkg->header.method = static_cast<ECodecMethod>(ntohs(static_cast<uint16_t>(header.method)));

    pkg->header.id = ntohl(header.id);
    pkg->header.length = ntohll(header.length);

    if (pkg->header.length == 0)
        co_return;

    pkg->data.Resize(pkg->header.length);
    co_await async_read(mConn->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}
