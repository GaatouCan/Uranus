#include "Object.h"
#include "Reflect/ClazzMethod.h"

UObject::UObject() {
}

UObject::~UObject() {
}

void UObject::Invoke(const std::string &name, void *ret, void *param) {
    const auto *clazz = GetClazz();
    if (!clazz)
        return;

    const auto *method = clazz->FindMethod(name);
    if (!method)
        return;

    method->Invoke(this, ret, param);
}
