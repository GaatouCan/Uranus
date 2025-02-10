#pragma once

#include "../Package.h"
#include "../ByteArray.h"

#include <sstream>
#include <yaml-cpp/yaml.h>

static constexpr uint32_t kInvalidPackageId = 1000;


enum class ECodecMethod : int16_t {
    INVALID = 0,
    BASE_LINE = 1,
    PROTOBUF = 2,
    MAX_METHOD = 3
};

class BASE_API FPackage final : public IPackage {

    friend class UPackageCodecImpl;
    friend class UCrossRouteCodecImpl;

    struct FHeader {
        int32_t magic;
        int32_t version;

        int16_t method;
        int16_t reversh;

        int32_t id;
        int64_t length;
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

    FPackage(int32_t id, std::string_view str);
    FPackage(int32_t id, const std::stringstream &ss);

    void Reset() override;

    void Invalid() override;
    [[nodiscard]] bool IsAvailable() const override;

    FPackage &SetPackageID(int32_t id);
    [[nodiscard]] int32_t GetPackageID() const override;

    void CopyFrom(IPackage *other) override;

    FPackage &SetData(std::string_view str);
    FPackage &SetData(const std::stringstream &ss);

    FPackage &SetMagic(int32_t magic);
    [[nodiscard]] int32_t GetMagic() const;

    FPackage &SetVersion(int32_t version);
    [[nodiscard]] int32_t GetVersion() const;

    FPackage &SetMethod(ECodecMethod method);
    [[nodiscard]] ECodecMethod GetMethod() const;

    [[nodiscard]] size_t GetDataLength() const;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] const FByteArray &GetByteArray() const;

    static void SetPackageMagic(int32_t magic);
    static void SetPackageVersion(int32_t version);
    static void SetPackageMethod(const std::string &method);

    static void LoadConfig(const YAML::Node &config);

    static IPackage *CreatePackage();
    static void InitPackage(IPackage *pkg);

    static constexpr size_t kHeaderSize = sizeof(FHeader);

    static int32_t sPackageMagic;
    static int32_t sPackageVersion;
    static std::string sPackageMethod;

private:
    [[nodiscard]] FByteArray &RawByteArray();
};
