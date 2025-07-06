#include "Clazz.h"
#include "ClazzField.h"
#include "ClazzMethod.h"

#include <ranges>

UClazz::UClazz() {
}

UClazz::~UClazz() {
    for (const auto &field: mFieldMap | std::views::values) {
        delete field;
    }

    for (const auto &method: mMethodMap | std::views::values) {
        delete method;
    }
}

IClazzField *UClazz::FindField(const std::string &name) const {
    const auto iter = mFieldMap.find(name);
    return iter != mFieldMap.end() ? iter->second : nullptr;
}

IClazzMethod *UClazz::FindMethod(const std::string &name) const {
    const auto iter = mMethodMap.find(name);
    return iter != mMethodMap.end() ? iter->second : nullptr;
}

void UClazz::RegisterField(IClazzField *field) {
    if (field == nullptr) return;
    mFieldMap.insert_or_assign(field->GetName(), field);
}

void UClazz::RegisterMethod(IClazzMethod *method) {
    if (method == nullptr) return;
    mMethodMap.insert_or_assign(method->GetName(), method);
}
