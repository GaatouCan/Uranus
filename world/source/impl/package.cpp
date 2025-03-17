#include "../../include/impl/package.h"

#ifdef __linux__
#include <cstring>
#endif


FPackage::FPackage(IRecycler *handle)
    : IPackage(handle),
      header_() {

    memset(&header_, 0, sizeof(header_));
}

FPackage::~FPackage() = default;

bool FPackage::unused() const {
    return header_.id == (MINIMUM_PACKAGE_ID - 1);
}

void FPackage::initial() {
    header_.id = MINIMUM_PACKAGE_ID - 1;
}

bool FPackage::copyFrom(IRecyclable *other) {
    if (IRecyclable::copyFrom(other)) {
        if (const auto temp = dynamic_cast<FPackage *>(other); temp != nullptr) {
            memcpy(&header_, &temp->header_, sizeof(header_));
            payload_ = temp->payload_;
            header_.length = temp->header_.length;
            return true;
        }
    }
    return false;
}

void FPackage::reset() {
    header_.id = 0;
    payload_.reset();
}

bool FPackage::available() const {
    if (unused())
        return true;

    return header_.id >= MINIMUM_PACKAGE_ID && header_.id <= MAXIMUM_PACKAGE_ID;
}

FPackage &FPackage::setPackageID(const uint32_t id) {
    header_.id = id;
    return *this;
}

FPackage &FPackage::setData(const std::string_view str) {
    payload_.resize(str.size());
    memcpy(payload_.data(), str.data(), str.size());
    header_.length = str.size();
    return *this;
}

FPackage &FPackage::setData(const std::stringstream &ss) {
    return setData(ss.str());
}

FPackage &FPackage::setMagic(const uint32_t magic) {
    header_.magic = magic;
    return *this;
}

FPackage &FPackage::setVersion(const uint32_t version) {
    header_.version = version;
    return *this;
}

uint32_t FPackage::getMagic() const {
    return header_.magic;
}

uint32_t FPackage::getVersion() const {
    return header_.version;
}

FPackage &FPackage::setMethod(const ECodecMethod method) {
    header_.method = method;
    return *this;
}

ECodecMethod FPackage::getMethod() const {
    return header_.method;
}

uint32_t FPackage::getPackageID() const {
    return header_.id;
}

size_t FPackage::getDataLength() const {
    return payload_.size();
}

std::string FPackage::toString() const {
    return {payload_.begin(), payload_.end()};
}

const FByteArray &FPackage::getByteArray() const {
    return payload_;
}

FByteArray &FPackage::rawByteArray() {
    return payload_;
}
