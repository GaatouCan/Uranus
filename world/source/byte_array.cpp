#include "../include/byte_array.h"

ByteArray::ByteArray(const size_t size)
    : mData(size) {
}

ByteArray::ByteArray(const std::vector<uint8_t> &bytes)
    : mData(bytes) {
}

ByteArray::operator std::vector<unsigned char>() const {
    return mData;
}

void ByteArray::Reset() {
    mData.clear();
    mData.shrink_to_fit();
}

size_t ByteArray::Size() const {
    return mData.size();
}

void ByteArray::Resize(const size_t size) {
    mData.resize(size);
}

uint8_t * ByteArray::Data() {
    return mData.data();
}

std::vector<uint8_t> & ByteArray::GetRawRef() {
    return mData;
}

auto ByteArray::Begin() -> decltype(mData)::iterator {
    return mData.begin();
}

auto ByteArray::End() -> decltype(mData)::iterator {
    return mData.end();
}

auto ByteArray::Begin() const -> decltype(mData)::const_iterator {
    return mData.begin();
}

auto ByteArray::End() const -> decltype(mData)::const_iterator {
    return mData.end();
}

uint8_t ByteArray::operator[](const size_t pos) const noexcept {
    return mData[pos];
}
