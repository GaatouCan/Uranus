#pragma once

#include "../package.h"
#include "../byte_array.h"

#include <sstream>
#include <yaml-cpp/yaml.h>

static constexpr uint32_t MINIMUM_PACKAGE_ID = 1001;
static constexpr uint32_t MAXIMUM_PACKAGE_ID = 999999;


enum class ECodecMethod : uint16_t {
    BASE_LINE = 0,
    PROTOBUF = 1,
};

class BASE_API FPackage final : public IPackage {

    friend class UPackageCodec;
    // friend class UCrossRouteCodecImpl;

    struct FHeader {
        uint32_t magic;
        uint32_t version;

        ECodecMethod method;
        uint16_t reversh;

        uint32_t id;
        size_t length;
    };

    FHeader header_;
    FByteArray data_;

public:
    FPackage();
    ~FPackage() override;

    FPackage(const FPackage &rhs);
    FPackage(FPackage &&rhs) noexcept;

    FPackage &operator=(const FPackage &rhs);
    FPackage &operator=(FPackage &&rhs) noexcept;

    FPackage(uint32_t id, std::string_view str);
    FPackage(uint32_t id, const std::stringstream &ss);

    void reset() override;

    void invalid() override;
    [[nodiscard]] bool available() const override;

    FPackage &setPackageID(uint32_t id);
    [[nodiscard]] uint32_t getPackageID() const override;

    void copyFrom(IPackage *other) override;

    FPackage &setData(std::string_view str);
    FPackage &setData(const std::stringstream &ss);

    FPackage &setMagic(uint32_t magic);
    [[nodiscard]] uint32_t getMagic() const;

    FPackage &setVersion(uint32_t version);
    [[nodiscard]] uint32_t getVersion() const;

    FPackage &setMethod(ECodecMethod method);
    [[nodiscard]] ECodecMethod getMethod() const;

    [[nodiscard]] size_t getDataLength() const;

    [[nodiscard]] std::string toString() const;
    [[nodiscard]] const FByteArray &getByteArray() const;

    static void SetPackageMagic(uint32_t magic);
    static void SetPackageVersion(uint32_t version);
    static void SetPackageMethod(const std::string &method);

    static void LoadConfig(const YAML::Node &config);

    static IPackage *CreatePackage();
    static void InitPackage(IPackage *pkg);

    static constexpr size_t PACKAGE_HEADER_SIZE = sizeof(FHeader);

    static uint32_t kPackageMagic;
    static uint32_t kPackageVersion;
    static std::string kPackageMethod;

private:
    [[nodiscard]] FByteArray &rawByteArray();
};
