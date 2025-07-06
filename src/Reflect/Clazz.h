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

#define STATIC_CLASS_IMPL(clazz) \
    static UGenerated_##clazz *StaticClazz() { return &UGenerated_##clazz::Instance(); } \
    UClazz *GetClazz() const override { return StaticClazz(); }

#define GENERATED_CLAZZ_HEADER(clazz) \
    [[nodiscard]] constexpr const char *GetClazzName() const override { return #clazz; } \
    static UGenerated_Player &Instance(); \
    [[nodiscard]] UObject *Create() const override; \
    void Destroy(UObject *obj) const override; \
    [[nodiscard]] size_t GetClazzSize() const override;


#define REGISTER_FIELD(clazz, field) \
RegisterField(new TClazzField<clazz, decltype(clazz::field)>(#field, offsetof(clazz, field)));

#define REGISTER_METHOD(clazz, method) \
RegisterMethod(new TClazzMethod(#method, &##clazz##::##method));
