#include "../../include/impl/PackageCodecImpl.h"
#include "../../include/impl/Package.h"
#include "../../include/Connection.h"


#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif


UPackageCodecImpl::UPackageCodecImpl(UConnection *conn)
    : TPackageCodec(conn) {
}

awaitable<void> UPackageCodecImpl::EncodeT(FPackage *pkg) {
    FPackage::FHeader header{};

    header.magic = htonl(pkg->header_.magic);
    header.version = htonl(pkg->header_.version);
    header.method = static_cast<ECodecMethod>(htons(static_cast<uint16_t>(pkg->header_.method)));

    header.id = htonl(pkg->header_.id);
    header.length = htonll(pkg->header_.length);

    if (const auto len = co_await async_write(conn_->GetSocket(), asio::buffer(&header, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Write package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->header_.length == 0)
        co_return;

    co_await async_write(conn_->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}

awaitable<void> UPackageCodecImpl::DecodeT(FPackage *pkg) {
    if (const auto len = co_await async_read(conn_->GetSocket(), asio::buffer(&pkg->header_, FPackage::kHeaderSize)); len == 0) {
        spdlog::warn("{} - Read package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    pkg->header_.magic = ntohl(pkg->header_.magic);
    pkg->header_.version = ntohl(pkg->header_.version);
    pkg->header_.method = static_cast<ECodecMethod>(ntohs(static_cast<uint16_t>(pkg->header_.method)));

    pkg->header_.id = ntohl(pkg->header_.id);
    pkg->header_.length = ntohll(pkg->header_.length);

    if (pkg->header_.length == 0)
        co_return;

    pkg->data_.Resize(pkg->header_.length);
    co_await async_read(conn_->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}
