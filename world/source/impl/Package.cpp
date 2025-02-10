#include "../../include/impl/Package.h"

int32_t FPackage::sPackageMagic = 20250122;
int32_t FPackage::sPackageVersion = 1001;
std::string FPackage::sPackageMethod = "PROTOBUF";


FPackage::FPackage()
    : header_() {
    memset(&header_, 0, sizeof(header_));
}

FPackage::~FPackage() = default;

FPackage::FPackage(const FPackage &rhs) : FPackage() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = static_cast<int64_t>(data_.Size());
    }
}

FPackage::FPackage(FPackage &&rhs) noexcept : FPackage() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = static_cast<int64_t>(data_.Size());
    }
}

FPackage &FPackage::operator=(const FPackage &rhs) {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = static_cast<int64_t>(data_.Size());
    }
    return *this;
}

FPackage &FPackage::operator=(FPackage &&rhs) noexcept {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = static_cast<int64_t>(data_.Size());
    }
    return *this;
}

FPackage::FPackage(const int32_t id, const std::string_view str)
    : FPackage() {
    header_.id = id;
    SetData(str);
}

FPackage::FPackage(const int32_t id, const std::stringstream &ss)
    : FPackage(id, ss.str()) {
}

void FPackage::Reset() {
    memset(&header_, 0, sizeof(header_));
    data_.Reset();
}

void FPackage::Invalid() {
    header_.id = kInvalidPackageId;
}

bool FPackage::IsAvailable() const {
    return header_.id > kInvalidPackageId;
}

FPackage &FPackage::SetPackageID(const int32_t id) {
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

FPackage &FPackage::SetMagic(const int32_t magic) {
    header_.magic = magic;
    return *this;
}

FPackage &FPackage::SetVersion(const int32_t version) {
    header_.version = version;
    return *this;
}

int32_t FPackage::GetMagic() const {
    return header_.magic;
}

int32_t FPackage::GetVersion() const {
    return header_.version;
}

FPackage &FPackage::SetMethod(const ECodecMethod method) {
    header_.method = static_cast<int16_t>(method);
    return *this;
}

ECodecMethod FPackage::GetMethod() const {
    if (header_.method < 0 || header_.method >= static_cast<int16_t>(ECodecMethod::MAX_METHOD))
        return ECodecMethod::INVALID;

    return static_cast<ECodecMethod>(header_.method);
}

int32_t FPackage::GetPackageID() const {
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

void FPackage::SetPackageMagic(const int32_t magic) {
    sPackageMagic = magic;
}

void FPackage::SetPackageVersion(const int32_t version) {
    sPackageVersion = version;
}

void FPackage::SetPackageMethod(const std::string &method) {
    sPackageMethod = method;
}

void FPackage::LoadConfig(const YAML::Node &config) {
    if (config["package"].IsNull())
        return;

    if (!config["package"]["magic"].IsNull())
        SetPackageMagic(config["package"]["magic"].as<int>());

    if (!config["package"]["version"].IsNull())
        SetPackageVersion(config["package"]["version"].as<int>());


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

    temp->SetMagic(sPackageMagic);
    temp->SetVersion(sPackageVersion);

    if (sPackageMethod == "LineBased")
        temp->SetMethod(ECodecMethod::BASE_LINE);
    if (sPackageMethod == "Protobuf")
        temp->SetMethod(ECodecMethod::PROTOBUF);
}

FByteArray &FPackage::RawByteArray() {
    return data_;
}
