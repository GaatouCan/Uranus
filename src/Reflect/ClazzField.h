#pragma once

#include "../common.h"

#include <string>
#include <optional>

class UObject;

class BASE_API FClazzField final {

public:
    FClazzField() = delete;

    FClazzField(std::string name, std::string type, size_t offset);
    ~FClazzField();

    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] std::string GetType() const;
    [[nodiscard]] size_t GetOffset() const;

    template<class Type>
    std::optional<Type> Get(UObject *obj) const;

    template<class Type>
    void Set(UObject *obj, const Type &val) const;

private:
    const std::string mName;
    const std::string mType;
    const size_t mOffset;
};


template<class Type>
inline std::optional<Type> FClazzField::Get(UObject *obj) const {
    if (obj == nullptr)
        return std::nullopt;

    Type result = *reinterpret_cast<Type *>(reinterpret_cast<unsigned char *>(obj) + mOffset);
    return std::optional<Type>(result);
}

template<class Type>
inline void FClazzField::Set(UObject *obj, const Type &val) const {
    if (obj == nullptr)
        return;
    *reinterpret_cast<Type *>(reinterpret_cast<unsigned char *>(obj) + mOffset) = val;
}

