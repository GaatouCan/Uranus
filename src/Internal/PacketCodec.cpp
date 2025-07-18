#include "PacketCodec.h"

#include <spdlog/spdlog.h>

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#include <endian.h>
#endif


UPacketCodec::UPacketCodec(ATcpSocket &socket)
    : TPackageCodec(socket) {
}

awaitable<bool> UPacketCodec::EncodeT(const std::shared_ptr<FPacket> &pkt) {
    FPacket::FHeader header{};

    header.magic = htonl(pkt->mHeader.magic);
    header.id = htonl(pkt->mHeader.id);

    header.source = static_cast<int32_t>(htonl(pkt->mHeader.source));
    header.target = static_cast<int32_t>(htonl(pkt->mHeader.target));

#if defined(_WIN32) || defined(_WIN64)
    header.length = htonll(pkt->mHeader.length);
#else
    header.length = htobe64(pkt->header_.length);
#endif

    if (pkt->mHeader.length > 0) {
        const auto buffers = {
            asio::buffer(&header, FPacket::PACKAGE_HEADER_SIZE),
            asio::buffer(pkt->RawRef(), pkt->mHeader.length)
        };

        const auto [ec, len] = co_await async_write(GetSocket(), buffers);

        if (ec) {
            SPDLOG_WARN("{:<20} - {}", __FUNCTION__, ec.message());
            co_return false;
        }

        if (len == 0) {
            SPDLOG_WARN("{:<20} - Packet Length Equal Zero", __FUNCTION__);
            co_return false;
        }

        co_return true;
    }

    const auto [ec, len] = co_await async_write(GetSocket(), asio::buffer(&header, FPacket::PACKAGE_HEADER_SIZE));

    if (ec) {
        SPDLOG_WARN("{:<20} - {}", __FUNCTION__, ec.message());
        co_return false;
    }

    if (len == 0) {
        SPDLOG_WARN("{:<20} - Packet Header Length Equal Zero", __FUNCTION__);
        co_return false;
    }

    co_return true;
}

awaitable<bool> UPacketCodec::DecodeT(const std::shared_ptr<FPacket> &pkt) {
    const auto [ec, len] = co_await async_read(GetSocket(), asio::buffer(&pkt->mHeader, FPacket::PACKAGE_HEADER_SIZE));

    if (ec) {
        SPDLOG_WARN("{:<20} - {}", __FUNCTION__, ec.message());
        co_return false;
    }

    if (len == 0) {
        SPDLOG_WARN("{:<20} - Package Header Length Equal Zero", __FUNCTION__);
        co_return false;
    }

    pkt->mHeader.magic = ntohl(pkt->mHeader.magic);
    pkt->mHeader.id = ntohl(pkt->mHeader.id);

    pkt->mHeader.source = static_cast<int32_t>(ntohl(pkt->mHeader.source));
    pkt->mHeader.target = static_cast<int32_t>(ntohl(pkt->mHeader.target));

#if defined(_WIN32) || defined(_WIN64)
    pkt->mHeader.length = ntohll(pkt->mHeader.length);
#else
    pkt->header_.length = be64toh(pkt->header_.length);
#endif

    if (pkt->mHeader.length == 0)
        co_return true;

    pkt->mPayload.Reserve(pkt->mHeader.length);

    const auto [payload_ec, payload_length] = co_await async_read(GetSocket(), asio::buffer(pkt->RawRef()));

    if (payload_ec) {
        SPDLOG_WARN("{:<20} - {}", __FUNCTION__, ec.message());
        co_return false;
    }

    if (payload_length == 0) {
        SPDLOG_WARN("{:<20} - Packet Payload Length Equal Zero But Header 's Length Is [{}]", __FUNCTION__, pkt->mHeader.length);
        co_return false;
    }

    co_return true;
}
