#include "ByteArray.h"

#ifdef __linux__
#include <algorithm>
#endif


FByteArray::FByteArray(const size_t size)
    : mBytes(size) {
}

FByteArray::FByteArray(const std::vector<std::byte> &bytes)
    : mBytes(bytes) {
}

FByteArray::operator std::vector<std::byte>() const {
    return mBytes;
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

void FByteArray::Reserve(size_t capacity) {
    mBytes.reserve(capacity);
}

std::byte *FByteArray::Data() {
    return mBytes.data();
}

const std::byte *FByteArray::Data() const {
    return mBytes.data();
}

std::vector<std::byte> &FByteArray::RawRef() {
    return mBytes;
}

void FByteArray::FromString(const std::string_view sv) {
    mBytes.reserve(sv.size());
    std::memcpy(mBytes.data(), sv.data(), sv.size());
}

std::string FByteArray::ToString() const {
    return { reinterpret_cast<const char*>(mBytes.data()), mBytes.size() };
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

std::byte FByteArray::operator[](const size_t pos) const noexcept {
    return mBytes[pos];
}
