#pragma once


#include "Package.h"
#include "Recycle.h"
#include "ByteArray.h"

#include <sstream>


// Define Minimum And Maximum Available Package ID

inline constexpr uint32_t MINIMUM_PACKAGE_ID = 1001;
inline constexpr uint32_t MAXIMUM_PACKAGE_ID = 999999;


/**
 * The Implement Of Package For Data Exchange;
 * Use The Structure Of Header Plus Data Part;
 * The Header Occupies 24 Bytes And Uses Big-Endian Transmission In Network
 */
class BASE_API FPacket final : public IPackageInterface, public IRecycleInterface {

    friend class UPacketCodec;

    /// Packet Header Define
    struct FHeader {
        uint32_t magic;
        uint32_t id;

        int32_t source;
        int32_t target;

        size_t length;
    };

    FHeader header_;
    FByteArray payload_;

protected:
    void OnCreate() override;
    void Initial() override;
    void Reset() override;

public:
    FPacket();
    ~FPacket() override;

    [[nodiscard]] bool IsUnused() const override;
    [[nodiscard]] bool IsAvailable() const override;

    void SetPackageID(uint32_t id) override;
    [[nodiscard]] uint32_t GetPackageID() const override;

    bool CopyFrom(IRecycleInterface *other) override;
    bool CopyFrom(const std::shared_ptr<IRecycleInterface> &other) override;

    FPacket &SetData(std::string_view str);
    FPacket &SetData(const std::stringstream &ss);

    FPacket &SetMagic(uint32_t magic);
    [[nodiscard]] uint32_t GetMagic() const;

    [[nodiscard]] size_t GetPayloadLength() const;

    void SetSource(int32_t source) override;
    [[nodiscard]] int32_t GetSource() const override;

    void SetTarget(int32_t target) override;
    [[nodiscard]] int32_t GetTarget() const override;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] const FByteArray &Bytes() const;

    static constexpr size_t PACKAGE_HEADER_SIZE = sizeof(FHeader);

private:
    [[nodiscard]] std::vector<std::byte> &RawRef();
};
