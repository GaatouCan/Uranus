#include "../../include/impl/Package.h"

uint32_t FPackage::sPackageMagic = 20250122;
uint32_t FPackage::sPackageVersion = 1001;
std::string FPackage::sPackageMethod = "PROTOBUF";


FPackage::FPackage()
    : header() {
    memset(&header, 0, sizeof(header));
}

FPackage::~FPackage() = default;

FPackage::FPackage(const FPackage &rhs) : FPackage() {
    if (this != &rhs) {
        memcpy(&header, &rhs.header, sizeof(header));

        data = rhs.data;
        header.length = static_cast<int32_t>(data.Size());
    }
}

FPackage::FPackage(FPackage &&rhs) noexcept : FPackage() {
    if (this != &rhs) {
        memcpy(&header, &rhs.header, sizeof(header));

        data = std::move(rhs.data);
        header.length = static_cast<int32_t>(data.Size());
    }
}

FPackage &FPackage::operator=(const FPackage &rhs) {
    if (this != &rhs) {
        memcpy(&header, &rhs.header, sizeof(header));

        data = rhs.data;
        header.length = static_cast<int32_t>(data.Size());
    }
    return *this;
}

FPackage &FPackage::operator=(FPackage &&rhs) noexcept {
    if (this != &rhs) {
        memcpy(&header, &rhs.header, sizeof(header));

        data = std::move(rhs.data);
        header.length = static_cast<int32_t>(data.Size());
    }
    return *this;
}

FPackage::FPackage(const uint32_t id, const std::string_view str)
    : FPackage() {
    header.id = id;
    SetData(str);
}

FPackage::FPackage(const uint32_t id, const std::stringstream &ss)
    : FPackage(id, ss.str()) {
}

void FPackage::Reset() {
    memset(&header, 0, sizeof(header));
    data.Reset();
}

void FPackage::Invalid() {
    header.id = kInvalidPackageId;
}

bool FPackage::IsAvailable() const {
    return header.id > kInvalidPackageId;
}

FPackage &FPackage::SetPackageID(const uint32_t id) {
    header.id = id;
    return *this;
}

FPackage &FPackage::SetData(const std::string_view str) {
    data.Resize(str.size());
    memcpy(data.Data(), str.data(), str.size());
    header.length = static_cast<int32_t>(str.size());
    return *this;
}

FPackage &FPackage::SetData(const std::stringstream &ss) {
    return SetData(ss.str());
}

FPackage &FPackage::SetMagic(const uint32_t magic) {
    header.magic = magic;
    return *this;
}

FPackage &FPackage::SetVersion(const uint32_t version) {
    header.version = version;
    return *this;
}

uint32_t FPackage::GetMagic() const {
    return header.magic;
}

uint32_t FPackage::GetVersion() const {
    return header.version;
}

FPackage &FPackage::SetMethod(const ECodecMethod method) {
    header.method = method;
    return *this;
}

ECodecMethod FPackage::GetMethod() const {
    return header.method;
}

uint32_t FPackage::GetPackageID() const {
    return header.id;
}

void FPackage::CopyFrom(IPackage *other) {
    if (const auto tmp = dynamic_cast<FPackage *>(other); tmp != nullptr && tmp != this) {
        *this = *tmp;
    }
}

size_t FPackage::GetDataLength() const {
    return data.Size();
}

std::string FPackage::ToString() const {
    return {data.Begin(), data.End()};
}

const FByteArray &FPackage::GetByteArray() const {
    return data;
}

void FPackage::SetPackageMagic(const uint32_t magic) {
    sPackageMagic = magic;
}

void FPackage::SetPackageVersion(const uint32_t version) {
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
    return data;
}
