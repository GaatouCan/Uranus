#include "Clazz.h"
#include "ClazzFactory.h"
#include "ClazzField.h"

#include <ranges>

UClazz::UClazz() {
}

UClazz::~UClazz() {
    for (const auto &field : mFieldMap | std::views::values) {
        delete field;
    }
}

// void UClazz::RegisterToFactory(UClazzFactory *factory) {
//     if (factory != nullptr) {
//         factory->RegisterClazz(this);
//     }
// }

FClazzField * UClazz::FindField(const std::string &name) const {
    const auto iter = mFieldMap.find(name);
    return iter != mFieldMap.end() ? iter->second : nullptr;
}

void UClazz::RegisterField(FClazzField *field) {
    if (field == nullptr) return;
    mFieldMap[field->GetName()] = field;
}
