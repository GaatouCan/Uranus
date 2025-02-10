#include "../include/ByteArray.h"

FByteArray::FByteArray(const size_t size)
    : bytes_(size) {
}

FByteArray::FByteArray(const std::vector<byte> &bytes)
    : bytes_(bytes) {
}

FByteArray::operator std::vector<unsigned char>() const {
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

byte * FByteArray::Data() {
    return bytes_.data();
}

std::vector<byte> & FByteArray::GetRawRef() {
    return bytes_;
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
