#include "../include/ByteArray.h"

FByteArray::FByteArray(const size_t size)
    : bytes(size) {
}

FByteArray::FByteArray(const std::vector<byte> &bytes)
    : bytes(bytes) {
}

FByteArray::operator std::vector<unsigned char>() const {
    return bytes;
}

void FByteArray::Reset() {
    bytes.clear();
    bytes.shrink_to_fit();
}

size_t FByteArray::Size() const {
    return bytes.size();
}

void FByteArray::Resize(const size_t size) {
    bytes.resize(size);
}

byte * FByteArray::Data() {
    return bytes.data();
}

std::vector<byte> & FByteArray::GetRawRef() {
    return bytes;
}

auto FByteArray::Begin() -> decltype(bytes)::iterator {
    return bytes.begin();
}

auto FByteArray::End() -> decltype(bytes)::iterator {
    return bytes.end();
}

auto FByteArray::Begin() const -> decltype(bytes)::const_iterator {
    return bytes.begin();
}

auto FByteArray::End() const -> decltype(bytes)::const_iterator {
    return bytes.end();
}
