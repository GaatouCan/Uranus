#pragma once

#include "../common.h"

#include <string>
#include <absl/container/flat_hash_map.h>


class UObject;
class IClazzField;
class IClazzMethod;
class UClazzFactory;


class BASE_API UClazz {

protected:
    UClazz();

public:
    virtual ~UClazz();

    DISABLE_COPY_MOVE(UClazz)

    [[nodiscard]] virtual const char *GetClazzName() const = 0;

    [[nodiscard]] virtual UObject *Create() const = 0;
    virtual void Destroy(UObject *obj) const = 0;

    [[nodiscard]] virtual size_t GetClazzSize() const = 0;

    [[nodiscard]] IClazzField *FindField(const std::string &name) const;
    [[nodiscard]] IClazzMethod *FindMethod(const std::string &name) const;

protected:
    void RegisterField(IClazzField *field);
    void RegisterMethod(IClazzMethod *method);

protected:
    absl::flat_hash_map<std::string, IClazzField *> mFieldMap;
    absl::flat_hash_map<std::string, IClazzMethod *> mMethodMap;
};

