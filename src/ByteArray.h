#pragma once

#include <stdexcept>
#include <vector>
#include <span>

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

    FByteArray(const FByteArray &rhs) = default;
    FByteArray &operator=(const FByteArray &rhs) = default;

    FByteArray(FByteArray &&rhs) noexcept = default;
    FByteArray &operator=(FByteArray &&rhs) noexcept = default;

    /** Clean The Data And Release The Space */
    void Reset();

    [[nodiscard]] size_t Size() const;
    void Resize(size_t size);

    [[nodiscard]] std::byte *Data();
    [[nodiscard]] const std::byte *Data() const;
    std::vector<std::byte> &RawRef();

    void FromString(std::string_view sv);
    std::string ToString() const;

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

        const void *src = nullptr;

        if constexpr (std::is_pointer_v<T>) {
            static_assert(!std::is_null_pointer_v<T>, "Null Pointer Pass To CastFrom");
            src = static_cast<const void *>(source);
        } else {
            src = static_cast<const void *>(&source);
        }

        std::memcpy(mBytes.data(), src, size);
    }

    template<typename T>
    requires kPODType<T>
    void CastFromVector(const std::vector<T> &source) {
        constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
        mBytes.resize(size * source.size());

        if constexpr (std::is_pointer_v<T>) {
            for (size_t idx = 0; idx < size; idx++) {
                std::memcpy(mBytes.data() + idx * size, static_cast<const void *>(source[idx]), size);
            }
        } else {
            std::memcpy(mBytes.data(), static_cast<const void *>(source.data()), size * size);
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

        void *dist = nullptr;

        if constexpr (std::is_pointer_v<T>) {
            static_assert(!std::is_null_pointer_v<T>, "Null Pointer Pass To CastTo");
            dist = static_cast<void *>(target);
        } else {
            dist = static_cast<void *>(&target);
        }

        std::memcpy(dist, mBytes.data(), size);
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

        std::memset(dist.data(), 0, length);
        std::memcpy(dist.data(), mBytes.data(), length);
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

    const void *src = nullptr;

    if constexpr (std::is_pointer_v<T>) {
        static_assert(!std::is_null_pointer_v<T>, "DataToByteArray Get Null Pointer");
        src = static_cast<const void *>(data);
    } else {
        src = static_cast<const void *>(&data);
    }

    std::memcpy(bytes.data(), src, size);

    return bytes;
}

template<typename T>
requires FByteArray::kPODType<T>
void ByteArrayToData(const std::vector<std::byte> &bytes, T &target) {
    constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
    if (size > bytes.size()) {
        throw std::runtime_error("FByteArray::CastTo - Overflow.");
    }

    void *dist = nullptr;

    if constexpr (std::is_pointer_v<T>) {
        dist = static_cast<void *>(&target);
    } else {
        dist = static_cast<void *>(&target);
    }

    std::memcpy(dist, bytes.data(), size);
}

template<typename T>
requires FByteArray::kPODType<T>
std::vector<std::byte> VectorToByteArray(const std::vector<T> &list) {
    std::vector<std::byte> bytes;

    constexpr auto size = std::is_pointer_v<T> ? sizeof(std::remove_pointer_t<T>) : sizeof(T);
    bytes.resize(size * list.size());

    if constexpr (std::is_pointer_v<T>) {
        for (size_t idx = 0; idx < size; idx++) {
            std::memcpy(bytes.data() + idx * size, static_cast<const void *>(list[idx]), size);
        }
    } else {
        std::memcpy(bytes.data(), static_cast<const void *>(list.data()), size * size);
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

    std::memset(dist.data(), 0, length);
    std::memcpy(dist.data(), static_cast<const void *>(src.data()), length);
}
