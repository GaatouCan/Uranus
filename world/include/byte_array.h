#pragma once

#include "common.h"

#include <vector>

#ifdef __linux__
#include <cstdint>
#endif


class BASE_API ByteArray final {

    std::vector<uint8_t> array_;

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

    auto Begin() -> decltype(array_)::iterator;
    auto End() -> decltype(array_)::iterator;

    [[nodiscard]] auto Begin() const -> decltype(array_)::const_iterator;
    [[nodiscard]] auto End() const -> decltype(array_)::const_iterator;

    [[nodiscard]] uint8_t operator[](size_t pos) const noexcept;

    template<typename T>
    static constexpr bool kPODType = std::is_pointer_v<T> ?
        std::is_trivial_v<std::remove_pointer_t<T>> && std::is_standard_layout_v<std::remove_pointer_t<T>> :
        std::is_trivial_v<T> && std::is_standard_layout_v<T>;

    template<typename T>
    requires kPODType<T>
    void CastFromData(T source) {
        constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
        array_.resize(size);

        if constexpr (std::is_pointer_v<T>) {
            memcpy(array_.data(), source, size);
        } else {
            memcpy(array_.data(), &source, size);
        }
    }

    template<typename T>
    requires kPODType<T>
    ByteArray &operator << (T source) {
        CastFromData(source);
        return *this;
    }

    template<typename T>
    requires kPODType<T>
    bool CastToData(T *target) const {
        const bool ret = array_.size() >= sizeof(std::remove_pointer_t<T>);
        const auto size = ret ?  sizeof(std::remove_pointer_t<T>) : array_.size();

        memset(target, 0, size);
        memcpy(target, array_.data(), size);

        return ret;
    }

    template<typename T>
    requires kPODType<T>
    ByteArray &operator >> (T *target) {
        CastToData(target);
        return *this;
    }

    template<typename T>
    requires kPODType<T>
    static ByteArray FromData(T data) {
        ByteArray uint8_ts;
        uint8_ts.CastFromData(std::forward<T>(data));
        return uint8_ts;
    }
};

template<typename T>
requires ByteArray::kPODType<T>
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
requires ByteArray::kPODType<T>
bool BASE_API ByteArrayToData(const std::vector<uint8_t> &uint8_ts, T *data) {
    const bool ret = uint8_ts.size() >= sizeof(std::remove_pointer_t<T>);
    const auto size = ret ? sizeof(std::remove_pointer_t<T>) : uint8_ts.size();

    memset(data, 0, size);
    memcpy(data, uint8_ts.data(), size);

    return ret;
}