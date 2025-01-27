#pragma once

#include <vector>

#include "common.h"


using byte = unsigned char;

class BASE_API FByteArray final {

    std::vector<byte> bytes;

public:
    FByteArray() = default;

    explicit FByteArray(size_t size);
    explicit FByteArray(const std::vector<byte> &bytes);

    explicit operator std::vector<byte>() const;

    void Reset();

    [[nodiscard]] size_t Size() const;
    void Resize(size_t size);

    [[nodiscard]] byte *Data();

    std::vector<byte> &GetRawRef();

    auto Begin() -> decltype(bytes)::iterator;
    auto End() -> decltype(bytes)::iterator;

    [[nodiscard]] auto Begin() const -> decltype(bytes)::const_iterator;
    [[nodiscard]] auto End() const -> decltype(bytes)::const_iterator;

    template<typename T>
    static constexpr bool CheckPODType = std::is_pointer_v<T> ?
        std::is_trivial_v<std::remove_pointer_t<T>> && std::is_standard_layout_v<std::remove_pointer_t<T>> :
        std::is_trivial_v<T> && std::is_standard_layout_v<T>;

    template<typename T>
    requires CheckPODType<T>
    void CastFromData(T source) {
        constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
        bytes.resize(size);

        if constexpr (std::is_pointer_v<T>) {
            memcpy(bytes.data(), source, size);
        } else {
            memcpy(bytes.data(), &source, size);
        }
    }

    template<typename T>
    requires CheckPODType<T>
    bool CastToData(T *target) const {
        const bool ret = bytes.size() >= sizeof(std::remove_pointer_t<T>);
        const auto size = ret ?  sizeof(std::remove_pointer_t<T>) : bytes.size();

        memset(target, 0, size);
        memcpy(target, bytes.data(), size);

        return ret;
    }

    template<typename T>
    requires CheckPODType<T>
    static FByteArray FromData(T data) {
        FByteArray bytes;
        bytes.CastFromData(std::forward<T>(data));
        return bytes;
    }
};

template<typename T>
requires FByteArray::CheckPODType<T>
std::vector<byte> BASE_API DataToByteArray(T data) {
    std::vector<byte> bytes;

    constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
    bytes.resize(size);

    if constexpr (std::is_pointer_v<T>) {
        memcpy(bytes.data(), data, size);
    } else {
        memcpy(bytes.data(), &data, size);
    }

    return bytes;
}

template<typename T>
requires FByteArray::CheckPODType<T>
bool BASE_API ByteArrayToData(const std::vector<byte> &bytes, T *data) {
    const bool ret = bytes.size() >= sizeof(std::remove_pointer_t<T>);
    const auto size = ret ? sizeof(std::remove_pointer_t<T>) : bytes.size();

    memset(data, 0, size);
    memcpy(data, bytes.data(), size);

    return ret;
}
