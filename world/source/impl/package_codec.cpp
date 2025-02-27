#include "../../include/impl/package_codec.h"
#include "../../include/impl/package.h"
#include "../../include/connection.h"


#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#include <endian.h>
#endif


PackageCodec::PackageCodec(const std::weak_ptr<Connection> &conn)
    : TPackageCodec(conn) {
}

awaitable<void> PackageCodec::EncodeT(Package *pkg) {
    Package::Header header{};

    header.magic = htonl(pkg->header_.magic);
    header.version = htonl(pkg->header_.version);
    header.method = static_cast<CodecMethod>(htons(static_cast<uint16_t>(pkg->header_.method)));

    header.id = htonl(pkg->header_.id);

#if defined(_WIN32) || defined(_WIN64)
    header.length = htonll(pkg->header_.length);
#else
    header.length = htobe64(pkg->mHeader.length);
#endif

    if (const auto len = co_await async_write(conn_.lock()->GetSocket(), asio::buffer(&header, Package::PACKAGE_HEADER_SIZE)); len == 0) {
        spdlog::warn("{} - Write package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    if (pkg->header_.length == 0)
        co_return;

    co_await async_write(conn_.lock()->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}

awaitable<void> PackageCodec::DecodeT(Package *pkg) {
    if (const auto len = co_await async_read(conn_.lock()->GetSocket(), asio::buffer(&pkg->header_, Package::PACKAGE_HEADER_SIZE)); len == 0) {
        spdlog::warn("{} - Read package header length equal zero", __FUNCTION__);
        pkg->Invalid();
        co_return;
    }

    pkg->header_.magic = ntohl(pkg->header_.magic);
    pkg->header_.version = ntohl(pkg->header_.version);
    pkg->header_.method = static_cast<CodecMethod>(ntohs(static_cast<uint16_t>(pkg->header_.method)));

    pkg->header_.id = ntohl(pkg->header_.id);

#if defined(_WIN32) || defined(_WIN64)
    pkg->header_.length = ntohll(pkg->header_.length);
#else
    pkg->mHeader.length = be64toh(pkg->mHeader.length);
#endif

    if (pkg->header_.length == 0)
        co_return;

    pkg->data_.Resize(pkg->header_.length);
    co_await async_read(conn_.lock()->GetSocket(), asio::buffer(pkg->RawByteArray().GetRawRef()));
}
