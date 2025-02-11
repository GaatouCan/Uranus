#include "../../include/impl/Package.h"

uint32_t Package::package_magic_ = 20250122;
uint32_t Package::package_version_ = 1001;
std::string Package::package_method_ = "PROTOBUF";


Package::Package()
    : header_() {
    memset(&header_, 0, sizeof(header_));
}

Package::~Package() = default;

Package::Package(const Package &rhs) : Package() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = data_.Size();
    }
}

Package::Package(Package &&rhs) noexcept : Package() {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = data_.Size();
    }
}

Package &Package::operator=(const Package &rhs) {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = rhs.data_;
        header_.length = data_.Size();
    }
    return *this;
}

Package &Package::operator=(Package &&rhs) noexcept {
    if (this != &rhs) {
        memcpy(&header_, &rhs.header_, sizeof(header_));

        data_ = std::move(rhs.data_);
        header_.length = data_.Size();
    }
    return *this;
}

Package::Package(const uint32_t id, const std::string_view str)
    : Package() {
    header_.id = id;
    SetData(str);
}

Package::Package(const uint32_t id, const std::stringstream &ss)
    : Package(id, ss.str()) {
}

void Package::Reset() {
    memset(&header_, 0, sizeof(header_));
    data_.Reset();
}

void Package::Invalid() {
    header_.id = MINIMUM_PACKAGE_ID - 1;
}

bool Package::IsAvailable() const {
    return header_.id >= MINIMUM_PACKAGE_ID && header_.id <= MAXIMUM_PACKAGE_ID;
}

Package &Package::SetPackageID(const uint32_t id) {
    header_.id = id;
    return *this;
}

Package &Package::SetData(const std::string_view str) {
    data_.Resize(str.size());
    memcpy(data_.Data(), str.data(), str.size());
    header_.length = static_cast<int64_t>(str.size());
    return *this;
}

Package &Package::SetData(const std::stringstream &ss) {
    return SetData(ss.str());
}

Package &Package::SetMagic(const uint32_t magic) {
    header_.magic = magic;
    return *this;
}

Package &Package::SetVersion(const uint32_t version) {
    header_.version = version;
    return *this;
}

uint32_t Package::GetMagic() const {
    return header_.magic;
}

uint32_t Package::GetVersion() const {
    return header_.version;
}

Package &Package::SetMethod(const CodecMethod method) {
    header_.method = method;
    return *this;
}

CodecMethod Package::GetMethod() const {
    return header_.method;
}

uint32_t Package::GetPackageID() const {
    return header_.id;
}

void Package::CopyFrom(IPackage *other) {
    if (const auto tmp = dynamic_cast<Package *>(other); tmp != nullptr && tmp != this) {
        *this = *tmp;
    }
}

size_t Package::GetDataLength() const {
    return data_.Size();
}

std::string Package::ToString() const {
    return {data_.Begin(), data_.End()};
}

const ByteArray &Package::GetByteArray() const {
    return data_;
}

void Package::SetPackageMagic(const uint32_t magic) {
    package_magic_ = magic;
}

void Package::SetPackageVersion(const uint32_t version) {
    package_version_ = version;
}

void Package::SetPackageMethod(const std::string &method) {
    package_method_ = method;
}

void Package::LoadConfig(const YAML::Node &config) {
    if (config["package"].IsNull())
        return;

    if (!config["package"]["magic"].IsNull())
        SetPackageMagic(config["package"]["magic"].as<uint32_t>());

    if (!config["package"]["version"].IsNull())
        SetPackageVersion(config["package"]["version"].as<uint32_t>());


    if (!config["package"]["method"].IsNull())
        SetPackageMethod(config["package"]["method"].as<std::string>());
}

IPackage * Package::CreatePackage() {
    return new Package();
}

void Package::InitPackage(IPackage *pkg) {
    const auto temp = dynamic_cast<Package *>(pkg);
    if (temp == nullptr)
        return;

    temp->SetMagic(package_magic_);
    temp->SetVersion(package_version_);

    if (package_method_ == "LineBased")
        temp->SetMethod(CodecMethod::BASE_LINE);
    if (package_method_ == "Protobuf")
        temp->SetMethod(CodecMethod::PROTOBUF);
}

ByteArray &Package::RawByteArray() {
    return data_;
}
