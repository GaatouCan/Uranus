#include "../include/ByteArray.h"

ByteArray::ByteArray(const size_t size)
    : bytes_(size) {
}

ByteArray::ByteArray(const std::vector<byte> &bytes)
    : bytes_(bytes) {
}

ByteArray::operator std::vector<unsigned char>() const {
    return bytes_;
}

void ByteArray::Reset() {
    bytes_.clear();
    bytes_.shrink_to_fit();
}

size_t ByteArray::Size() const {
    return bytes_.size();
}

void ByteArray::Resize(const size_t size) {
    bytes_.resize(size);
}

byte * ByteArray::Data() {
    return bytes_.data();
}

std::vector<byte> & ByteArray::GetRawRef() {
    return bytes_;
}

auto ByteArray::Begin() -> decltype(bytes_)::iterator {
    return bytes_.begin();
}

auto ByteArray::End() -> decltype(bytes_)::iterator {
    return bytes_.end();
}

auto ByteArray::Begin() const -> decltype(bytes_)::const_iterator {
    return bytes_.begin();
}

auto ByteArray::End() const -> decltype(bytes_)::const_iterator {
    return bytes_.end();
}
