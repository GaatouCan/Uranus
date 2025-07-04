#include "ClazzField.h"

FClazzField::FClazzField(std::string name, std::string type, const size_t offset, const size_t size)
    : mName(std::move(name)),
      mType(std::move(type)),
      mOffset(offset),
      mSize(size) {
}

FClazzField::~FClazzField() {
}

std::string FClazzField::GetName() const {
    return mName;
}

std::string FClazzField::GetType() const {
    return mType;
}

size_t FClazzField::GetOffset() const {
    return mOffset;
}

size_t FClazzField::GetSize() const {
    return mSize;
}
