#include "../../include/impl/package.h"

#ifdef __linux__
#include <cstring>
#endif


uint32_t FPackage::kPackageMagic = 20250122;
uint32_t FPackage::kPackageVersion = 1001;
ECodecMethod FPackage::kPackageMethod = ECodecMethod::PROTOBUF;

FPackage::FPackage(IRecycler *handle)
    : IPackage(handle),
      header_() {

    memset(&header_, 0, sizeof(header_));
}

FPackage::~FPackage() = default;

void FPackage::initial() {
    header_.magic = kPackageMagic;
    header_.version = kPackageVersion;
    header_.method = kPackageMethod;
}

bool FPackage::copyFrom(IRecyclable *other) {
    if (IRecyclable::copyFrom(other)) {
        if (const auto temp = dynamic_cast<FPackage *>(other); temp != nullptr) {
            memcpy(&header_, &temp->header_, sizeof(header_));
            data_ = temp->data_;
            header_.length = temp->header_.length;
            return true;
        }
    }
    return false;
}

void FPackage::reset() {
    memset(&header_, 0, sizeof(header_));
    data_.reset();
}

bool FPackage::available() const {
    return header_.id >= MINIMUM_PACKAGE_ID && header_.id <= MAXIMUM_PACKAGE_ID;
}

FPackage &FPackage::setPackageID(const uint32_t id) {
    header_.id = id;
    return *this;
}

FPackage &FPackage::setData(const std::string_view str) {
    data_.resize(str.size());
    memcpy(data_.data(), str.data(), str.size());
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

void FPackage::invalid() {
    reset();
}

size_t FPackage::getDataLength() const {
    return data_.size();
}

std::string FPackage::toString() const {
    return {data_.begin(), data_.end()};
}

const FByteArray &FPackage::getByteArray() const {
    return data_;
}

FByteArray &FPackage::rawByteArray() {
    return data_;
}

void FPackage::LoadConfig(const YAML::Node &config) {
    if (config["package"].IsNull())
        return;

    if (!config["package"]["magic"].IsNull())
        kPackageMagic = config["package"]["magic"].as<uint32_t>();

    if (!config["package"]["version"].IsNull())
        kPackageVersion = config["package"]["version"].as<uint32_t>();


    if (!config["package"]["method"].IsNull()) {
        const auto str = config["package"]["method"].as<std::string>();
        if (str == "LineBased")
            kPackageMethod = ECodecMethod::BASE_LINE;
        if (str == "Protobuf")
            kPackageMethod = ECodecMethod::PROTOBUF;
    }
}
