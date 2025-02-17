#pragma once

#include <vector>

#ifdef __linux__
#include <cstdint>
#endif

#include "common.h"


class BASE_API ByteArray final {

    std::vector<uint8_t> mData;

public:
    ByteArray() = default;

    explicit ByteArray(size_t size);
    explicit ByteArray(const std::vector<uint8_t> &bytes);

    explicit operator std::vector<uint8_t>() const;

    void Reset();

    [[nodiscard]] size_t Size() const;
    void Resize(size_t size);

    [[nodiscard]] uint8_t *Data();

    std::vector<uint8_t> &GetRawRef();

    auto Begin() -> decltype(mData)::iterator;
    auto End() -> decltype(mData)::iterator;

    [[nodiscard]] auto Begin() const -> decltype(mData)::const_iterator;
    [[nodiscard]] auto End() const -> decltype(mData)::const_iterator;

    [[nodiscard]] uint8_t operator[](size_t pos) const noexcept;

    template<typename T>
    static constexpr bool KPODType = std::is_pointer_v<T> ?
        std::is_trivial_v<std::remove_pointer_t<T>> && std::is_standard_layout_v<std::remove_pointer_t<T>> :
        std::is_trivial_v<T> && std::is_standard_layout_v<T>;

    template<typename T>
    requires KPODType<T>
    void CastFromData(T source) {
        constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
        mData.resize(size);

        if constexpr (std::is_pointer_v<T>) {
            memcpy(mData.data(), source, size);
        } else {
            memcpy(mData.data(), &source, size);
        }
    }

    template<typename T>
    requires KPODType<T>
    bool CastToData(T *target) const {
        const bool ret = mData.size() >= sizeof(std::remove_pointer_t<T>);
        const auto size = ret ?  sizeof(std::remove_pointer_t<T>) : mData.size();

        memset(target, 0, size);
        memcpy(target, mData.data(), size);

        return ret;
    }

    template<typename T>
    requires KPODType<T>
    static ByteArray FromData(T data) {
        ByteArray uint8_ts;
        uint8_ts.CastFromData(std::forward<T>(data));
        return uint8_ts;
    }
};

template<typename T>
requires ByteArray::KPODType<T>
std::vector<uint8_t> BASE_API DataToByteArray(T data) {
    std::vector<uint8_t> uint8_ts;

    constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
    uint8_ts.resize(size);

    if constexpr (std::is_pointer_v<T>) {
        memcpy(uint8_ts.data(), data, size);
    } else {
        memcpy(uint8_ts.data(), &data, size);
    }

    return uint8_ts;
}

template<typename T>
requires ByteArray::KPODType<T>
bool BASE_API ByteArrayToData(const std::vector<uint8_t> &uint8_ts, T *data) {
    const bool ret = uint8_ts.size() >= sizeof(std::remove_pointer_t<T>);
    const auto size = ret ? sizeof(std::remove_pointer_t<T>) : uint8_ts.size();

    memset(data, 0, size);
    memcpy(data, uint8_ts.data(), size);

    return ret;
}