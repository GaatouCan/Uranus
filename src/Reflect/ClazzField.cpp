#include "ClazzField.h"

#include <utility>

IClazzField::IClazzField(std::string name, const size_t offset)
    : mName(std::move(name)),
      mOffset(offset) {
}

IClazzField::~IClazzField() {
}

std::string IClazzField::GetName() const {
    return mName;
}

size_t IClazzField::GetOffset() const {
    return mOffset;
}
