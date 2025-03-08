#include "../include/byte_array.h"

FByteArray::FByteArray(const size_t size)
    : array_(size) {
}

FByteArray::FByteArray(const std::vector<uint8_t> &bytes)
    : array_(bytes) {
}

FByteArray::operator std::vector<unsigned char>() const {
    return array_;
}

void FByteArray::reset() {
    array_.clear();
    array_.shrink_to_fit();
}

size_t FByteArray::size() const {
    return array_.size();
}

void FByteArray::resize(const size_t size) {
    array_.resize(size);
}

uint8_t * FByteArray::data() {
    return array_.data();
}

std::vector<uint8_t> & FByteArray::rawRef() {
    return array_;
}

auto FByteArray::begin() -> decltype(array_)::iterator {
    return array_.begin();
}

auto FByteArray::end() -> decltype(array_)::iterator {
    return array_.end();
}

auto FByteArray::begin() const -> decltype(array_)::const_iterator {
    return array_.begin();
}

auto FByteArray::end() const -> decltype(array_)::const_iterator {
    return array_.end();
}

uint8_t FByteArray::operator[](const size_t pos) const noexcept {
    return array_[pos];
}
