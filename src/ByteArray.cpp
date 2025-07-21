#include "ByteArray.h"

#ifdef __linux__
#include <algorithm>
#endif


FByteArray::FByteArray(const size_t size)
    : mByteArray(size) {
}

FByteArray::FByteArray(const std::vector<std::byte> &bytes)
    : mByteArray(bytes) {
}

FByteArray::operator std::vector<std::byte>() const {
    return mByteArray;
}

void FByteArray::Reset() {
    mByteArray.clear();
    mByteArray.shrink_to_fit();
}

size_t FByteArray::Size() const {
    return mByteArray.size();
}

void FByteArray::Resize(const size_t size) {
    mByteArray.resize(size);
}

void FByteArray::Reserve(size_t capacity) {
    mByteArray.reserve(capacity);
}

std::byte *FByteArray::Data() {
    return mByteArray.data();
}

const std::byte *FByteArray::Data() const {
    return mByteArray.data();
}

std::vector<std::byte> &FByteArray::RawRef() {
    return mByteArray;
}

void FByteArray::FromString(const std::string_view sv) {
    mByteArray.reserve(sv.size());
    std::memcpy(mByteArray.data(), sv.data(), sv.size());
}

std::string FByteArray::ToString() const {
    return { reinterpret_cast<const char*>(mByteArray.data()), mByteArray.size() };
}

auto FByteArray::Begin() -> decltype(mByteArray)::iterator {
    return mByteArray.begin();
}

auto FByteArray::End() -> decltype(mByteArray)::iterator {
    return mByteArray.end();
}

auto FByteArray::Begin() const -> decltype(mByteArray)::const_iterator {
    return mByteArray.begin();
}

auto FByteArray::End() const -> decltype(mByteArray)::const_iterator {
    return mByteArray.end();
}

std::byte FByteArray::operator[](const size_t pos) const noexcept {
    return mByteArray[pos];
}
