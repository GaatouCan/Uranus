#include "ByteArray.h"

#ifdef __linux__
#include <algorithm>
#endif


FByteArray::FByteArray(const size_t size)
    : bytes_(size) {
}

FByteArray::FByteArray(const std::vector<std::byte> &bytes)
    : bytes_(bytes) {
}

FByteArray::operator std::vector<std::byte>() const {
    return bytes_;
}

void FByteArray::Reset() {
    bytes_.clear();
    bytes_.shrink_to_fit();
}

size_t FByteArray::Size() const {
    return bytes_.size();
}

void FByteArray::Resize(const size_t size) {
    bytes_.resize(size);
}

void FByteArray::Reserve(size_t capacity) {
    bytes_.reserve(capacity);
}

std::byte *FByteArray::Data() {
    return bytes_.data();
}

const std::byte *FByteArray::Data() const {
    return bytes_.data();
}

std::vector<std::byte> &FByteArray::RawRef() {
    return bytes_;
}

void FByteArray::FromString(const std::string_view sv) {
    bytes_.reserve(sv.size());
    std::memcpy(bytes_.data(), sv.data(), sv.size());
}

std::string FByteArray::ToString() const {
    return { reinterpret_cast<const char*>(bytes_.data()), bytes_.size() };
}

auto FByteArray::Begin() -> decltype(bytes_)::iterator {
    return bytes_.begin();
}

auto FByteArray::End() -> decltype(bytes_)::iterator {
    return bytes_.end();
}

auto FByteArray::Begin() const -> decltype(bytes_)::const_iterator {
    return bytes_.begin();
}

auto FByteArray::End() const -> decltype(bytes_)::const_iterator {
    return bytes_.end();
}

std::byte FByteArray::operator[](const size_t pos) const noexcept {
    return bytes_[pos];
}
