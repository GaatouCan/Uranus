#include "../include/byte_array.h"

ByteArray::ByteArray(const size_t size)
    : array_(size) {
}

ByteArray::ByteArray(const std::vector<uint8_t> &bytes)
    : array_(bytes) {
}

ByteArray::operator std::vector<unsigned char>() const {
    return array_;
}

void ByteArray::Reset() {
    array_.clear();
    array_.shrink_to_fit();
}

size_t ByteArray::Size() const {
    return array_.size();
}

void ByteArray::Resize(const size_t size) {
    array_.resize(size);
}

uint8_t * ByteArray::Data() {
    return array_.data();
}

std::vector<uint8_t> & ByteArray::GetRawRef() {
    return array_;
}

auto ByteArray::Begin() -> decltype(array_)::iterator {
    return array_.begin();
}

auto ByteArray::End() -> decltype(array_)::iterator {
    return array_.end();
}

auto ByteArray::Begin() const -> decltype(array_)::const_iterator {
    return array_.begin();
}

auto ByteArray::End() const -> decltype(array_)::const_iterator {
    return array_.end();
}

uint8_t ByteArray::operator[](const size_t pos) const noexcept {
    return array_[pos];
}
