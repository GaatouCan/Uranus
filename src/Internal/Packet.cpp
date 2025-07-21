#include "Packet.h"

#ifdef __linux__
#include <cstring>
#endif


inline constexpr int PACKET_MAGIC = 20250514;

FPacket::FPacket()
    : header_() {
    memset(&header_, 0, sizeof(header_));
}

FPacket::~FPacket() = default;

bool FPacket::IsUnused() const {
    // As Same As Assigned In Initial() Method
    return header_.id == (MINIMUM_PACKAGE_ID - 1);
}

void FPacket::OnCreate() {
    SetMagic(PACKET_MAGIC);
}

void FPacket::Initial() {
    header_.id = MINIMUM_PACKAGE_ID - 1;
    header_.source = -1;
    header_.target = -1;
}

bool FPacket::CopyFrom(IRecycleInterface *other) {
    if (IRecycleInterface::CopyFrom(other)) {
        if (const auto temp = dynamic_cast<FPacket *>(other); temp != nullptr) {
            memcpy(&header_, &temp->header_, sizeof(header_));
            payload_ = temp->payload_;
            header_.length = temp->header_.length;
            return true;
        }
    }
    return false;
}

bool FPacket::CopyFrom(const std::shared_ptr<IRecycleInterface> &other) {
    if (IRecycleInterface::CopyFrom(other)) {
        if (const auto temp = std::dynamic_pointer_cast<FPacket>(other); temp != nullptr) {
            memcpy(&header_, &temp->header_, sizeof(header_));
            payload_ = temp->payload_;
            header_.length = temp->header_.length;
            return true;
        }
    }
    return false;
}

void FPacket::Reset() {
    header_.id = 0;
    payload_.Reset();
}

bool FPacket::IsAvailable() const {
    if (IsUnused())
        return true;

    return header_.id >= MINIMUM_PACKAGE_ID && header_.id <= MAXIMUM_PACKAGE_ID;
}

void FPacket::SetPackageID(const uint32_t id) {
    header_.id = id;
}

FPacket &FPacket::SetData(const std::string_view str) {
    header_.length = str.size();
    payload_.FromString(str);
    return *this;
}

FPacket &FPacket::SetData(const std::stringstream &ss) {
    return SetData(ss.str());
}

FPacket &FPacket::SetMagic(const uint32_t magic) {
    header_.magic = magic;
    return *this;
}

uint32_t FPacket::GetMagic() const {
    return header_.magic;
}

uint32_t FPacket::GetPackageID() const {
    return header_.id;
}

size_t FPacket::GetPayloadLength() const {
    return payload_.Size();
}

void FPacket::SetSource(const int32_t source) {
    header_.source = source;
}

int32_t FPacket::GetSource() const {
    return header_.source;
}

void FPacket::SetTarget(const int32_t target) {
    header_.target = target;
}

int32_t FPacket::GetTarget() const {
    return header_.target;
}

std::string FPacket::ToString() const {
    return payload_.ToString();
}

const FByteArray &FPacket::Bytes() const {
    return payload_;
}

std::vector<std::byte> &FPacket::RawRef() {
    return payload_.RawRef();
}
