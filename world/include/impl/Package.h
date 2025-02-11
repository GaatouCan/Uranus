#pragma once

#include "../package.h"
#include "../byte_array.h"

#include <sstream>
#include <yaml-cpp/yaml.h>

static constexpr uint32_t MINIMUM_PACKAGE_ID = 1001;
static constexpr uint32_t MAXIMUM_PACKAGE_ID = 999999;


enum class CodecMethod : uint16_t {
    BASE_LINE = 0,
    PROTOBUF = 1,
};

class BASE_API Package final : public IPackage {

    friend class PackageCodec;
    friend class UCrossRouteCodecImpl;

    struct Header {
        uint32_t magic;
        uint32_t version;

        CodecMethod method;
        uint16_t reversh;

        uint32_t id;
        size_t length;
    };

    Header mHeader;
    ByteArray mData;

public:
    Package();
    ~Package() override;

    Package(const Package &rhs);
    Package(Package &&rhs) noexcept;

    Package &operator=(const Package &rhs);
    Package &operator=(Package &&rhs) noexcept;

    Package(uint32_t id, std::string_view str);
    Package(uint32_t id, const std::stringstream &ss);

    void Reset() override;

    void Invalid() override;
    [[nodiscard]] bool IsAvailable() const override;

    Package &SetPackageID(uint32_t id);
    [[nodiscard]] uint32_t GetPackageID() const override;

    void CopyFrom(IPackage *other) override;

    Package &SetData(std::string_view str);
    Package &SetData(const std::stringstream &ss);

    Package &SetMagic(uint32_t magic);
    [[nodiscard]] uint32_t GetMagic() const;

    Package &SetVersion(uint32_t version);
    [[nodiscard]] uint32_t GetVersion() const;

    Package &SetMethod(CodecMethod method);
    [[nodiscard]] CodecMethod GetMethod() const;

    [[nodiscard]] size_t GetDataLength() const;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] const ByteArray &GetByteArray() const;

    static void SetPackageMagic(uint32_t magic);
    static void SetPackageVersion(uint32_t version);
    static void SetPackageMethod(const std::string &method);

    static void LoadConfig(const YAML::Node &config);

    static IPackage *CreatePackage();
    static void InitPackage(IPackage *pkg);

    static constexpr size_t PACKAGE_HEADER_SIZE = sizeof(Header);

    static uint32_t kPackageMagic;
    static uint32_t kPackageVersion;
    static std::string kPackageMethod;

private:
    [[nodiscard]] ByteArray &RawByteArray();
};
