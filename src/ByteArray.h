#pragma once

#include <stdexcept>
#include <vector>

#include "common.h"

#ifdef __linux__
#include <cstdint>
#endif

/**
 * The Wrapper Of std::vector<unsigned char>
 */
class BASE_API FByteArray final {

    std::vector<std::byte> mBytes;

public:
    FByteArray() = default;

    explicit FByteArray(size_t size);
    explicit FByteArray(const std::vector<std::byte> &bytes);

    explicit operator std::vector<std::byte>() const;

    FByteArray(const FByteArray &rhs);
    FByteArray &operator=(const FByteArray &rhs);

    FByteArray(FByteArray &&rhs) noexcept;
    FByteArray &operator=(FByteArray &&rhs) noexcept;

    /** Clean The Data And Release The Space */
    void Reset();

    [[nodiscard]] size_t Size() const;
    void Resize(size_t size);

    [[nodiscard]] std::byte *Data();
    [[nodiscard]] const std::byte *Data() const;
    std::vector<std::byte> &RawRef();

    auto Begin() -> decltype(mBytes)::iterator;
    auto End() -> decltype(mBytes)::iterator;

    [[nodiscard]] auto Begin() const -> decltype(mBytes)::const_iterator;
    [[nodiscard]] auto End() const -> decltype(mBytes)::const_iterator;

    [[nodiscard]] std::byte operator[](size_t pos) const noexcept;

    template<typename T>
    static constexpr bool kPODType = std::is_pointer_v<T>
        ? std::is_trivial_v<std::remove_pointer_t<T> > && std::is_standard_layout_v<std::remove_pointer_t<T> >
        : std::is_trivial_v<T> && std::is_standard_layout_v<T>;

    template<typename T>
    requires kPODType<T>
    void CastFrom(const T &source) {
        constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
        mBytes.resize(size);

        if constexpr (std::is_pointer_v<T>) {
            memcpy(static_cast<void *>(mBytes.data()), source, size);
        } else {
            memcpy(static_cast<void *>(mBytes.data()), &source, size);
        }
    }

    template<typename T>
    requires kPODType<T>
    void CastFromVector(const std::vector<T> &source) {
        constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
        mBytes.resize(size * source.size());

        if constexpr (std::is_pointer_v<T>) {
            for (size_t idx = 0; idx < size; idx++) {
                memcpy(static_cast<void *>(mBytes.data()) + idx * size, source[idx], size);
            }
        } else {
            memcpy(static_cast<void *>(mBytes.data()), source.data(), mBytes.size());
        }
    }

    template<typename T>
    requires kPODType<T>
    FByteArray &operator <<(const T &source) {
        this->CastFrom(source);
        return *this;
    }

    template<typename T>
    requires kPODType<T>
    FByteArray &operator <<(const std::vector<T> &source) {
        this->CastFromVector(source);
        return *this;
    }

    template<typename T>
    requires kPODType<T>
    void CastTo(T &target) const {
        constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
        if (size > mBytes.size()) {
            throw std::runtime_error("FByteArray::CastTo - Overflow.");
        }

        if constexpr (std::is_pointer_v<T>) {
            memcpy(target, static_cast<const void *>(mBytes.data()), size);
        } else {
            memcpy(&target, static_cast<const void *>(mBytes.data()), size);
        }
    }

    template<typename T>
    requires (!std::is_pointer_v<T>) && std::is_trivial_v<T> && std::is_standard_layout_v<T>
    void CastToVector(std::vector<T> &dist) {
        constexpr auto size = sizeof(T);
        const size_t count = mBytes.size() / size;
        const size_t length = size * count;

        if (count == 0)
            return;

        dist.resize(count);
        memset(dist.data(), 0, length);
        memcpy(dist.data(), static_cast<const void *>(mBytes.data()), length);
    }

    template<typename T>
    requires kPODType<T>
    FByteArray &operator >>(T *target) {
        this->CastTo(target);
        return *this;
    }

    template<typename T>
    requires (!std::is_pointer_v<T>) && std::is_trivial_v<T> && std::is_standard_layout_v<T>
    FByteArray &operator >>(std::vector<T> &dist) {
        this->CastToVector(dist);
        return *this;
    }

    template<typename T>
    requires kPODType<T>
    static FByteArray From(T &&data) {
        FByteArray bytes;
        bytes.CastFrom(std::forward<T>(data));
        return bytes;
    }
};

template<typename T>
requires FByteArray::kPODType<T>
std::vector<std::byte> DataToByteArray(T data) {
    std::vector<std::byte> bytes;

    constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
    bytes.resize(size);

    if constexpr (std::is_pointer_v<T>) {
        memcpy(static_cast<void *>(bytes.data()), data, size);
    } else {
        memcpy(static_cast<void *>(bytes.data()), &data, size);
    }

    return bytes;
}

template<typename T>
requires FByteArray::kPODType<T>
void ByteArrayToData(const std::vector<std::byte> &bytes, T &target) {
    constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
    if (size > bytes.size()) {
        throw std::runtime_error("FByteArray::CastTo - Overflow.");
    }

    if constexpr (std::is_pointer_v<T>) {
        memcpy(target, static_cast<const void *>(bytes.data()), size);
    } else {
        memcpy(&target, static_cast<const void *>(bytes.data()), size);
    }
}

template<typename T>
requires FByteArray::kPODType<T>
std::vector<std::byte> VectorToByteArray(const std::vector<T> &list) {
    std::vector<std::byte> bytes;

    constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
    bytes.resize(size * list.size());

    if constexpr (std::is_pointer_v<T>) {
        for (size_t idx = 0; idx < size; idx++) {
            memcpy(static_cast<void *>(bytes.data()) + idx * size, list[idx], size);
        }
    } else {
        memcpy(static_cast<void *>(bytes.data()), list.data(), bytes.size());
    }

    return bytes;
}

template<typename T>
requires (!std::is_pointer_v<T>) && std::is_trivial_v<T> && std::is_standard_layout_v<T>
void ByteArrayToVector(const std::vector<std::byte> &src, std::vector<T> &dist) {
    constexpr auto size = sizeof(T);
    const size_t count = src.size() / size;
    const size_t length = size * count;

    if (count == 0)
        return;

    dist.resize(count);
    memset(dist.data(), 0, length);
    memcpy(dist.data(), static_cast<const void *>(src.data()), length);
}
