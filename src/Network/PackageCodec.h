#pragma once

#include "Package.h"
#include "Types.h"


class BASE_API IPackageCodec {

protected:
    explicit IPackageCodec(ATcpSocket &socket);

public:
    virtual ~IPackageCodec() = default;

    DISABLE_COPY_MOVE(IPackageCodec)

    virtual awaitable<bool> Encode(const std::shared_ptr<IPackage> &pkg) = 0;
    virtual awaitable<bool> Decode(const std::shared_ptr<IPackage> &pkg) = 0;

    [[nodiscard]] ATcpSocket &GetSocket() const;

private:
    ATcpSocket &mSocket;
};

template<CPackageType Type>
class TPackageCodec : public IPackageCodec {

protected:
    explicit TPackageCodec(ATcpSocket &socket)
        : IPackageCodec(socket) {
    }

public:
    using PackageType = Type;

    ~TPackageCodec() override = default;

    awaitable<bool> Encode(const std::shared_ptr<IPackage> &pkg) override {
        if (auto temp = std::dynamic_pointer_cast<Type>(pkg); temp != nullptr) {
            const auto ret = co_await this->EncodeT(temp);
            co_return ret;
        }
        co_return false;
    }

    awaitable<bool> Decode(const std::shared_ptr<IPackage> &pkg) override {
        if (auto temp = std::dynamic_pointer_cast<Type>(pkg); temp != nullptr) {
            const auto ret = co_await this->DecodeT(temp);
            co_return ret;
        }
        co_return false;
    }

    virtual awaitable<bool> EncodeT(const std::shared_ptr<Type> &pkg) = 0;
    virtual awaitable<bool> DecodeT(const std::shared_ptr<Type> &pkg) = 0;
};
