#include "../../include/impl/package_codec.h"
#include "../../include/impl/package.h"
#include "../../include/connection.h"


#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#include <endian.h>
#endif


UPackageCodec::UPackageCodec(const std::weak_ptr<UConnection> &conn)
    : TPackageCodec(conn) {
}

awaitable<void> UPackageCodec::encodeT(FPackage *pkg) {
    FPackage::FHeader header{};

    header.magic = htonl(pkg->header_.magic);
    header.version = htonl(pkg->header_.version);
    header.method = static_cast<ECodecMethod>(htons(static_cast<uint16_t>(pkg->header_.method)));

    header.id = htonl(pkg->header_.id);

#if defined(_WIN32) || defined(_WIN64)
    header.length = htonll(pkg->header_.length);
#else
    header.length = htobe64(pkg->header_.length);
#endif

    if (const auto len = co_await async_write(conn_.lock()->getSocket(), asio::buffer(&header, FPackage::PACKAGE_HEADER_SIZE)); len == 0) {
        spdlog::warn("{} - Write package header length equal zero", __FUNCTION__);
        pkg->invalid();
        co_return;
    }

    if (pkg->header_.length == 0)
        co_return;

    co_await async_write(conn_.lock()->getSocket(), asio::buffer(pkg->rawByteArray().rawRef()));
}

awaitable<void> UPackageCodec::decodeT(FPackage *pkg) {
    if (const auto len = co_await async_read(conn_.lock()->getSocket(), asio::buffer(&pkg->header_, FPackage::PACKAGE_HEADER_SIZE)); len == 0) {
        spdlog::warn("{} - Read package header length equal zero", __FUNCTION__);
        pkg->invalid();
        co_return;
    }

    pkg->header_.magic = ntohl(pkg->header_.magic);
    pkg->header_.version = ntohl(pkg->header_.version);
    pkg->header_.method = static_cast<ECodecMethod>(ntohs(static_cast<uint16_t>(pkg->header_.method)));

    pkg->header_.id = ntohl(pkg->header_.id);

#if defined(_WIN32) || defined(_WIN64)
    pkg->header_.length = ntohll(pkg->header_.length);
#else
    pkg->header_.length = be64toh(pkg->header_.length);
#endif

    if (pkg->header_.length == 0)
        co_return;

    pkg->data_.resize(pkg->header_.length);
    co_await async_read(conn_.lock()->getSocket(), asio::buffer(pkg->rawByteArray().rawRef()));
}
