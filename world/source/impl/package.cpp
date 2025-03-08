#include "../../include/impl/package.h"

#ifdef __linux__
#include <cstring>
#endif


uint32_t FPackage::kPackageMagic = 20250122;
uint32_t FPackage::kPackageVersion = 1001;
std::string FPackage::kPackageMethod = "PROTOBUF";


FPackage::FPackage()
    : header_() {
    memset(&header_, 0, sizeof(header_));
}

FPackage::~FPackage() = default;

FPackage::FPackage(const FPackage &rhs) : FPackage() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = data_.size();
    }
}

FPackage::FPackage(FPackage &&rhs) noexcept : FPackage() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = data_.size();
    }
}

FPackage &FPackage::operator=(const FPackage &rhs) {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = data_.size();
    }
    return *this;
}

FPackage &FPackage::operator=(FPackage &&rhs) noexcept {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = data_.size();
    }
    return *this;
}

FPackage::FPackage(const uint32_t id, const std::string_view str)
    : FPackage() {
    header_.id = id;
    setData(str);
}

FPackage::FPackage(const uint32_t id, const std::stringstream &ss)
    : FPackage(id, ss.str()) {
}

void FPackage::reset() {
    memset(&header_, 0, sizeof(header_));
    data_.reset();
}

void FPackage::invalid() {
    header_.id = MINIMUM_PACKAGE_ID - 1;
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

void FPackage::copyFrom(IPackage *other) {
    if (const auto tmp = dynamic_cast<FPackage *>(other); tmp != nullptr && tmp != this) {
        *this = *tmp;
    }
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

void FPackage::SetPackageMagic(const uint32_t magic) {
    kPackageMagic = magic;
}

void FPackage::SetPackageVersion(const uint32_t version) {
    kPackageVersion = version;
}

void FPackage::SetPackageMethod(const std::string &method) {
    kPackageMethod = method;
}

void FPackage::LoadConfig(const YAML::Node &config) {
    if (config["package"].IsNull())
        return;

    if (!config["package"]["magic"].IsNull())
        SetPackageMagic(config["package"]["magic"].as<uint32_t>());

    if (!config["package"]["version"].IsNull())
        SetPackageVersion(config["package"]["version"].as<uint32_t>());


    if (!config["package"]["method"].IsNull())
        SetPackageMethod(config["package"]["method"].as<std::string>());
}

IPackage * FPackage::CreatePackage() {
    return new FPackage();
}

void FPackage::InitPackage(IPackage *pkg) {
    const auto temp = dynamic_cast<FPackage *>(pkg);
    if (temp == nullptr)
        return;

    temp->setMagic(kPackageMagic);
    temp->setVersion(kPackageVersion);

    if (kPackageMethod == "LineBased")
        temp->setMethod(ECodecMethod::BASE_LINE);
    if (kPackageMethod == "Protobuf")
        temp->setMethod(ECodecMethod::PROTOBUF);
}

FByteArray &FPackage::rawByteArray() {
    return data_;
}
