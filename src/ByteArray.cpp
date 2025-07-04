#include "ByteArray.h"

#ifdef __linux__
#include <algorithm>
#endif


FByteArray::FByteArray(const size_t size)
    : mBytes(size) {
}

FByteArray::FByteArray(const std::vector<uint8_t> &bytes)
    : mBytes(bytes) {
}

FByteArray::operator std::vector<unsigned char>() const {
    return mBytes;
}

FByteArray::FByteArray(const FByteArray &rhs) {
    mBytes.resize(rhs.mBytes.size());
    std::ranges::copy(rhs.mBytes, mBytes.begin());
}

FByteArray &FByteArray::operator=(const FByteArray &rhs) {
    if (this != &rhs) {
        mBytes.resize(rhs.mBytes.size());
        std::ranges::copy(rhs.mBytes, mBytes.begin());
    }
    return *this;
}

FByteArray::FByteArray(FByteArray &&rhs) noexcept {
    mBytes = std::move(rhs.mBytes);
}

FByteArray &FByteArray::operator=(FByteArray &&rhs) noexcept {
    if (this != &rhs) {
        mBytes = std::move(rhs.mBytes);
    }
    return *this;
}

void FByteArray::Reset() {
    mBytes.clear();
    mBytes.shrink_to_fit();
}

size_t FByteArray::Size() const {
    return mBytes.size();
}

void FByteArray::Resize(const size_t size) {
    mBytes.resize(size);
}

uint8_t *FByteArray::Data() {
    return mBytes.data();
}

std::vector<uint8_t> &FByteArray::RawRef() {
    return mBytes;
}

auto FByteArray::Begin() -> decltype(mBytes)::iterator {
    return mBytes.begin();
}

auto FByteArray::End() -> decltype(mBytes)::iterator {
    return mBytes.end();
}

auto FByteArray::Begin() const -> decltype(mBytes)::const_iterator {
    return mBytes.begin();
}

auto FByteArray::End() const -> decltype(mBytes)::const_iterator {
    return mBytes.end();
}

uint8_t FByteArray::operator[](const size_t pos) const noexcept {
    return mBytes[pos];
}
