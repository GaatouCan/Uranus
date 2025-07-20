#include "ByteArray.h"

#ifdef __linux__
#include <algorithm>
#endif


FByteArray::FByteArray(const size_t size)
    : Bytes(size) {
}

FByteArray::FByteArray(const std::vector<std::byte> &bytes)
    : Bytes(bytes) {
}

FByteArray::operator std::vector<std::byte>() const {
    return Bytes;
}

void FByteArray::Reset() {
    Bytes.clear();
    Bytes.shrink_to_fit();
}

size_t FByteArray::Size() const {
    return Bytes.size();
}

void FByteArray::Resize(const size_t size) {
    Bytes.resize(size);
}

void FByteArray::Reserve(size_t capacity) {
    Bytes.reserve(capacity);
}

std::byte *FByteArray::Data() {
    return Bytes.data();
}

const std::byte *FByteArray::Data() const {
    return Bytes.data();
}

std::vector<std::byte> &FByteArray::RawRef() {
    return Bytes;
}

void FByteArray::FromString(const std::string_view sv) {
    Bytes.reserve(sv.size());
    std::memcpy(Bytes.data(), sv.data(), sv.size());
}

std::string FByteArray::ToString() const {
    return { reinterpret_cast<const char*>(Bytes.data()), Bytes.size() };
}

auto FByteArray::Begin() -> decltype(Bytes)::iterator {
    return Bytes.begin();
}

auto FByteArray::End() -> decltype(Bytes)::iterator {
    return Bytes.end();
}

auto FByteArray::Begin() const -> decltype(Bytes)::const_iterator {
    return Bytes.begin();
}

auto FByteArray::End() const -> decltype(Bytes)::const_iterator {
    return Bytes.end();
}

std::byte FByteArray::operator[](const size_t pos) const noexcept {
    return Bytes[pos];
}
