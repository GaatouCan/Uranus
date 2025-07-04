#include "ClazzField.h"

FClazzField::FClazzField(std::string name, std::string type, const size_t offset)
    : mName(std::move(name)),
      mType(std::move(type)),
      mOffset(offset) {
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
