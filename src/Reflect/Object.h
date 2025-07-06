#pragma once

#include "Clazz.h"


class BASE_API UObject {

public:
    UObject();
    virtual ~UObject();

    bool SetFieldValue(const std::string &name, void *value);
    bool GetFieldValue(const std::string &name, void *ret) const;

    bool InvokeMethod(const std::string& name, void *ret, void *param);

    template<class Type>
    bool SetField(const std::string &name, Type && value);

    template<class Type>
    std::tuple<Type, bool> GetField(const std::string &name) const;

    template<class ... Args>
    bool Invoke(const std::string& name, void *ret, Args&& ... args);

protected:
    [[nodiscard]] virtual UClazz *GetClazz() const = 0;
};


template<class Type>
inline bool UObject::SetField(const std::string &name, Type &&value) {
    return this->SetFieldValue(name, static_cast<void*>(const_cast<std::remove_cv_t<Type>*>(&value)));
}

template<class Type>
inline std::tuple<Type, bool> UObject::GetField(const std::string &name) const {
    Type result;
    if (this->GetFieldValue(name, &result))
        return { result, true };
    return { result, false };
}

template<class ... Args>
inline bool UObject::Invoke(const std::string &name, void *ret, Args &&...args) {
    using TupleType = std::tuple<std::decay_t<Args>...>;
    TupleType param = std::make_tuple(std::forward<Args>(args)...);
    return this->InvokeMethod(name, ret, reinterpret_cast<void *>(&param));
}

#define COMBINE_BODY_MACRO_IMPL(A, B, C, D) A##B##C##D
#define COMBINE_BODY_MACRO(A, B, C, D) COMBINE_BODY_MACRO_IMPL(A, B, C, D)

#define GENERATED_BODY() \
    COMBINE_BODY_MACRO(CURRENT_FILE_ID, _, __LINE__, _GENERATED_IMPL)

#define COMBINE_BODY_MACRO_IMPL(A, B, C, D) A##B##C##D

#define COMBINE_BODY_MACRO(A, B, C, D) COMBINE_BODY_MACRO_IMPL(A, B, C, D)

#define GENERATED_BODY() \
    COMBINE_BODY_MACRO(CURRENT_FILE_ID, _, __LINE__, _GENERATED_IMPL)
