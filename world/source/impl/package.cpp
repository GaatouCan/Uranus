#include "../../include/impl/package.h"


uint32_t Package::kPackageMagic = 20250122;
uint32_t Package::kPackageVersion = 1001;
std::string Package::kPackageMethod = "PROTOBUF";


Package::Package()
    : mHeader() {
    memset(&mHeader, 0, sizeof(mHeader));
}

Package::~Package() = default;

Package::Package(const Package &rhs) : Package() {
    if (this != &rhs) {
        memcpy(&mHeader, &rhs.mHeader, sizeof(mHeader));

        mData = rhs.mData;
        mHeader.length = mData.Size();
    }
}

Package::Package(Package &&rhs) noexcept : Package() {
    if (this != &rhs) {
        memcpy(&mHeader, &rhs.mHeader, sizeof(mHeader));

        mData = std::move(rhs.mData);
        mHeader.length = mData.Size();
    }
}

Package &Package::operator=(const Package &rhs) {
    if (this != &rhs) {
        memcpy(&mHeader, &rhs.mHeader, sizeof(mHeader));

        mData = rhs.mData;
        mHeader.length = mData.Size();
    }
    return *this;
}

Package &Package::operator=(Package &&rhs) noexcept {
    if (this != &rhs) {
        memcpy(&mHeader, &rhs.mHeader, sizeof(mHeader));

        mData = std::move(rhs.mData);
        mHeader.length = mData.Size();
    }
    return *this;
}

Package::Package(const uint32_t id, const std::string_view str)
    : Package() {
    mHeader.id = id;
    SetData(str);
}

Package::Package(const uint32_t id, const std::stringstream &ss)
    : Package(id, ss.str()) {
}

void Package::Reset() {
    memset(&mHeader, 0, sizeof(mHeader));
    mData.Reset();
}

void Package::Invalid() {
    mHeader.id = MINIMUM_PACKAGE_ID - 1;
}

bool Package::IsAvailable() const {
    return mHeader.id >= MINIMUM_PACKAGE_ID && mHeader.id <= MAXIMUM_PACKAGE_ID;
}

Package &Package::SetPackageID(const uint32_t id) {
    mHeader.id = id;
    return *this;
}

Package &Package::SetData(const std::string_view str) {
    mData.Resize(str.size());
    memcpy(mData.Data(), str.data(), str.size());
    mHeader.length = static_cast<int64_t>(str.size());
    return *this;
}

Package &Package::SetData(const std::stringstream &ss) {
    return SetData(ss.str());
}

Package &Package::SetMagic(const uint32_t magic) {
    mHeader.magic = magic;
    return *this;
}

Package &Package::SetVersion(const uint32_t version) {
    mHeader.version = version;
    return *this;
}

uint32_t Package::GetMagic() const {
    return mHeader.magic;
}

uint32_t Package::GetVersion() const {
    return mHeader.version;
}

Package &Package::SetMethod(const CodecMethod method) {
    mHeader.method = method;
    return *this;
}

CodecMethod Package::GetMethod() const {
    return mHeader.method;
}

uint32_t Package::GetPackageID() const {
    return mHeader.id;
}

void Package::CopyFrom(IPackage *other) {
    if (const auto tmp = dynamic_cast<Package *>(other); tmp != nullptr && tmp != this) {
        *this = *tmp;
    }
}

size_t Package::GetDataLength() const {
    return mData.Size();
}

std::string Package::ToString() const {
    return {mData.Begin(), mData.End()};
}

const ByteArray &Package::GetByteArray() const {
    return mData;
}

void Package::SetPackageMagic(const uint32_t magic) {
    kPackageMagic = magic;
}

void Package::SetPackageVersion(const uint32_t version) {
    kPackageVersion = version;
}

void Package::SetPackageMethod(const std::string &method) {
    kPackageMethod = method;
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

    temp->SetMagic(kPackageMagic);
    temp->SetVersion(kPackageVersion);

    if (kPackageMethod == "LineBased")
        temp->SetMethod(CodecMethod::BASE_LINE);
    if (kPackageMethod == "Protobuf")
        temp->SetMethod(CodecMethod::PROTOBUF);
}

ByteArray &Package::RawByteArray() {
    return mData;
}
