#pragma once

#include "../Package.h"
#include "../ByteArray.h"

#include <sstream>
#include <yaml-cpp/yaml.h>

static constexpr uint32_t kInvalidPackageId = 1000;


enum class ECodecMethod : uint16_t {
    BASE_LINE = 1,
    PROTOBUF = 2
};

class BASE_API FPackage final : public IPackage {

    friend class UPackageCodecImpl;
    friend class UCrossRouteCodecImpl;

    struct FHeader {
        uint32_t magic;
        uint32_t version;

        ECodecMethod method;
        uint16_t reversh;

        uint32_t id;
        size_t length;
    };

    FHeader header;
    FByteArray data;

public:
    FPackage();
    ~FPackage() override;

    FPackage(const FPackage &rhs);
    FPackage(FPackage &&rhs) noexcept;

    FPackage &operator=(const FPackage &rhs);
    FPackage &operator=(FPackage &&rhs) noexcept;

    FPackage(uint32_t id, std::string_view str);
    FPackage(uint32_t id, const std::stringstream &ss);

    void Reset() override;

    void Invalid() override;
    [[nodiscard]] bool IsAvailable() const override;

    FPackage &SetPackageID(uint32_t id);
    [[nodiscard]] uint32_t GetPackageID() const override;

    void CopyFrom(IPackage *other) override;

    FPackage &SetData(std::string_view str);
    FPackage &SetData(const std::stringstream &ss);

    FPackage &SetMagic(uint32_t magic);
    [[nodiscard]] uint32_t GetMagic() const;

    FPackage &SetVersion(uint32_t version);
    [[nodiscard]] uint32_t GetVersion() const;

    FPackage &SetMethod(ECodecMethod method);
    [[nodiscard]] ECodecMethod GetMethod() const;

    [[nodiscard]] size_t GetDataLength() const;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] const FByteArray &GetByteArray() const;

    static void SetPackageMagic(uint32_t magic);
    static void SetPackageVersion(uint32_t version);
    static void SetPackageMethod(const std::string &method);

    static void LoadConfig(const YAML::Node &config);

    static IPackage *CreatePackage();
    static void InitPackage(IPackage *pkg);

    static constexpr size_t kHeaderSize = sizeof(FHeader);

    static uint32_t sPackageMagic;
    static uint32_t sPackageVersion;
    static std::string sPackageMethod;

private:
    [[nodiscard]] FByteArray &RawByteArray();
};
