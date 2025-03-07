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

void FByteArray::Reset() {
    array_.clear();
    array_.shrink_to_fit();
}

size_t FByteArray::Size() const {
    return array_.size();
}

void FByteArray::Resize(const size_t size) {
    array_.resize(size);
}

uint8_t * FByteArray::Data() {
    return array_.data();
}

std::vector<uint8_t> & FByteArray::GetRawRef() {
    return array_;
}

auto FByteArray::Begin() -> decltype(array_)::iterator {
    return array_.begin();
}

auto FByteArray::End() -> decltype(array_)::iterator {
    return array_.end();
}

auto FByteArray::Begin() const -> decltype(array_)::const_iterator {
    return array_.begin();
}

auto FByteArray::End() const -> decltype(array_)::const_iterator {
    return array_.end();
}

uint8_t FByteArray::operator[](const size_t pos) const noexcept {
    return array_[pos];
}
