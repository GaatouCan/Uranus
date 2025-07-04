#pragma once

#include "Reflect/Clazz.h"
#include "Reflect/ClazzField.h"
#include "Reflect/ClazzMethod.h"

class BASE_API UObject {

public:
    UObject();
    virtual ~UObject();

    template<class Type>
    std::optional<Type> GetField(const std::string& name);

    template<class Type>
    void SetField(const std::string& name, const Type &value);

    template<class FuncType, class... Args>
    std::optional<std::invoke_result_t<FuncType, Args ...>> Invoke(Args && ... args);

protected:
    [[nodiscard]] virtual UClazz *GetClazz() const = 0;
};

template<class Type>
inline std::optional<Type> UObject::GetField(const std::string &name) {
    const auto *clazz = this->GetClazz();
    if (clazz == nullptr)
        return std::nullopt;

    const auto *field = clazz->FindField(name);
    if (field == nullptr)
        return std::nullopt;

    return field->Get<Type>(this);
}

template<class Type>
inline void UObject::SetField(const std::string &name, const Type &value) {
    const auto *clazz = this->GetClazz();
    if (clazz == nullptr)
        return;

    const auto *field = clazz->FindField(name);
    if (field == nullptr)
        return;

    field->Set<Type>(this, value);
}

template<class FuncType, class ... Args>
inline std::optional<std::invoke_result_t<FuncType, Args...>> UObject::Invoke(Args &&...args) {
    const auto *clazz = this->GetClazz();
    if (clazz == nullptr)
        return;
}
