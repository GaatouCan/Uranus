#include "../../include/impl/Package.h"

uint32_t FPackage::packageMagic = 20250122;
uint32_t FPackage::packageVersion = 1001;
std::string FPackage::packageMethod = "PROTOBUF";


FPackage::FPackage()
    : header_() {
    memset(&header_, 0, sizeof(header_));
}

FPackage::~FPackage() = default;

FPackage::FPackage(const FPackage &rhs) : FPackage() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = data_.Size();
    }
}

FPackage::FPackage(FPackage &&rhs) noexcept : FPackage() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = data_.Size();
    }
}

FPackage &FPackage::operator=(const FPackage &rhs) {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = data_.Size();
    }
    return *this;
}

FPackage &FPackage::operator=(FPackage &&rhs) noexcept {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = data_.Size();
    }
    return *this;
}

FPackage::FPackage(const uint32_t id, const std::string_view str)
    : FPackage() {
    header_.id = id;
    SetData(str);
}

FPackage::FPackage(const uint32_t id, const std::stringstream &ss)
    : FPackage(id, ss.str()) {
}

void FPackage::Reset() {
    memset(&header_, 0, sizeof(header_));
    data_.Reset();
}

void FPackage::Invalid() {
    header_.id = MINIMUM_PACKAGE_ID - 1;
}

bool FPackage::IsAvailable() const {
    return header_.id >= MINIMUM_PACKAGE_ID && header_.id <= MAXIMUM_PACKAGE_ID;
}

FPackage &FPackage::SetPackageID(const uint32_t id) {
    header_.id = id;
    return *this;
}

FPackage &FPackage::SetData(const std::string_view str) {
    data_.Resize(str.size());
    memcpy(data_.Data(), str.data(), str.size());
    header_.length = static_cast<int64_t>(str.size());
    return *this;
}

FPackage &FPackage::SetData(const std::stringstream &ss) {
    return SetData(ss.str());
}

FPackage &FPackage::SetMagic(const uint32_t magic) {
    header_.magic = magic;
    return *this;
}

FPackage &FPackage::SetVersion(const uint32_t version) {
    header_.version = version;
    return *this;
}

uint32_t FPackage::GetMagic() const {
    return header_.magic;
}

uint32_t FPackage::GetVersion() const {
    return header_.version;
}

FPackage &FPackage::SetMethod(const ECodecMethod method) {
    header_.method = method;
    return *this;
}

ECodecMethod FPackage::GetMethod() const {
    return header_.method;
}

uint32_t FPackage::GetPackageID() const {
    return header_.id;
}

void FPackage::CopyFrom(IPackage *other) {
    if (const auto tmp = dynamic_cast<FPackage *>(other); tmp != nullptr && tmp != this) {
        *this = *tmp;
    }
}

size_t FPackage::GetDataLength() const {
    return data_.Size();
}

std::string FPackage::ToString() const {
    return {data_.Begin(), data_.End()};
}

const FByteArray &FPackage::GetByteArray() const {
    return data_;
}

void FPackage::SetPackageMagic(const uint32_t magic) {
    packageMagic = magic;
}

void FPackage::SetPackageVersion(const uint32_t version) {
    packageVersion = version;
}

void FPackage::SetPackageMethod(const std::string &method) {
    packageMethod = method;
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

    temp->SetMagic(packageMagic);
    temp->SetVersion(packageVersion);

    if (packageMethod == "LineBased")
        temp->SetMethod(ECodecMethod::BASE_LINE);
    if (packageMethod == "Protobuf")
        temp->SetMethod(ECodecMethod::PROTOBUF);
}

FByteArray &FPackage::RawByteArray() {
    return data_;
}
