#include "ClazzMethod.h"

#include <utility>

FClazzMethod::FClazzMethod(std::string name, const uintptr_t ptr)
    : mName(std::move(name)),
      mMethodPtr(ptr) {
}

FClazzMethod::~FClazzMethod() {
}

std::string FClazzMethod::GetName() const {
    return mName;
}

uintptr_t FClazzMethod::GetPointer() const {
    return mMethodPtr;
}
