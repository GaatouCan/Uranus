#pragma once

#include "Package.h"
#include "Types.h"


class BASE_API IPackageCodecBase {

protected:
    explicit IPackageCodecBase(ATcpSocket &socket);

public:
    virtual ~IPackageCodecBase() = default;

    DISABLE_COPY_MOVE(IPackageCodecBase)

    virtual awaitable<bool> Encode(const std::shared_ptr<IPackageInterface> &pkg) = 0;
    virtual awaitable<bool> Decode(const std::shared_ptr<IPackageInterface> &pkg) = 0;

    [[nodiscard]] ATcpSocket &GetSocket() const;

private:
    ATcpSocket &socket_;
};

template<CPackageType Type>
class TPackageCodec : public IPackageCodecBase {

protected:
    explicit TPackageCodec(ATcpSocket &socket)
        : IPackageCodecBase(socket) {
    }

public:
    using PackageType = Type;

    ~TPackageCodec() override = default;

    awaitable<bool> Encode(const std::shared_ptr<IPackageInterface> &pkg) override {
        if (auto temp = std::dynamic_pointer_cast<Type>(pkg); temp != nullptr) {
            const auto ret = co_await this->EncodeT(temp);
            co_return ret;
        }
        co_return false;
    }

    awaitable<bool> Decode(const std::shared_ptr<IPackageInterface> &pkg) override {
        if (auto temp = std::dynamic_pointer_cast<Type>(pkg); temp != nullptr) {
            const auto ret = co_await this->DecodeT(temp);
            co_return ret;
        }
        co_return false;
    }

    virtual awaitable<bool> EncodeT(const std::shared_ptr<Type> &pkg) = 0;
    virtual awaitable<bool> DecodeT(const std::shared_ptr<Type> &pkg) = 0;
};
