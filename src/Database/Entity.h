#pragma once

#include "../ByteArray.h"

#include <mysqlx/xdevapi.h>

class BASE_API IEntity {

public:
    virtual ~IEntity() = default;

    [[nodiscard]] virtual const char *GetTableName() const = 0;

    [[nodiscard]] virtual bool ComparePrimaryKey(mysqlx::Row &row) const = 0;

    virtual mysqlx::RowResult Query(mysqlx::Table &table) = 0;
    mysqlx::RowResult Query(mysqlx::Schema &schema);

    virtual void Read(mysqlx::Row &row) = 0;

    virtual void Write(mysqlx::Table &table) = 0;
    virtual void Write(mysqlx::Schema &schema);

    virtual void Remove(mysqlx::Table &table) = 0;
    virtual void Remove(mysqlx::Schema &schema);
};

template<typename T>
concept CEntityType = std::derived_from<T, IEntity>;

#define DB_CAST_FROM_BLOB(field, value) \
{ \
const auto bytes = (value).getRawBytes(); \
field = FByteArray(std::vector(bytes.begin(), bytes.end())); \
}

#define DB_CAST_TO_BLOB(field) \
mysqlx::bytes((field).Data(), (field).Size())
