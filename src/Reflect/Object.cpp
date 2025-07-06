#include "Object.h"
#include "ClazzMethod.h"
#include "ClazzField.h"

UObject::UObject() {
}

UObject::~UObject() {
}

bool UObject::SetFieldValue(const std::string &name, void *value) {
    const auto *clazz = GetClazz();
    if (!clazz)
        return false;

    const auto *field = clazz->FindField(name);
    if (!field)
        return field;

    return field->SetValue(this, value);
}

bool UObject::GetFieldValue(const std::string &name, void *ret) const {
    const auto *clazz = GetClazz();
    if (!clazz)
        return false;

    const auto *field = clazz->FindField(name);
    if (!field)
        return false;

    return field->GetValue(this, ret);
}

bool UObject::InvokeMethod(const std::string &name, void *ret, void *param) {
    const auto *clazz = GetClazz();
    if (!clazz)
        return false;

    const auto *method = clazz->FindMethod(name);
    if (!method)
        return false;

    return method->Invoke(this, ret, param);
}
