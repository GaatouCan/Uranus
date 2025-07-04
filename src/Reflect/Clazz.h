#pragma once

#include "../common.h"

#include <absl/container/flat_hash_map.h>

class UObject;
class FClazzField;
class UClazzFactory;

class BASE_API UClazz {


protected:
    UClazz();

public:
    virtual ~UClazz();

    DISABLE_COPY_MOVE(UClazz)

    [[nodiscard]] virtual const char *GetClazzName() const = 0;

    // void RegisterToFactory(UClazzFactory *factory);

    [[nodiscard]] virtual UObject *Create() const = 0;
    virtual void Destroy(UObject *obj) const = 0;

    [[nodiscard]] virtual size_t GetClazzSize() const = 0;

    [[nodiscard]] FClazzField *FindField(const std::string &name) const;

protected:
    void RegisterField(FClazzField *field);

protected:
    absl::flat_hash_map<std::string, FClazzField *> mFieldMap;
};

