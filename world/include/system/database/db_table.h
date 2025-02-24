#pragma once

#include "../../common.h"
#include "../../byte_array.h"

#include <mysqlx/xdevapi.h>


class BASE_API IDBTable {

public:
    virtual ~IDBTable() = default;

    [[nodiscard]] virtual const char* GetTableName() const = 0;

    [[nodiscard]] virtual bool ComparePrimaryKey(mysqlx::Row &row) const = 0;

    virtual mysqlx::RowResult Query(mysqlx::Table &table) = 0;
    virtual mysqlx::RowResult Query(mysqlx::Schema &schema) = 0;

    virtual void Read(mysqlx::Row &row) = 0;

    virtual void Write(mysqlx::Table &table) = 0;
    virtual void Write(mysqlx::Schema &schema) = 0;

    virtual void Remove(mysqlx::Table &table) = 0;
    virtual void Remove(mysqlx::Schema &schema) = 0;
};

template <typename T>
concept DBTableType = std::derived_from<T, IDBTable>;

#define DB_CAST_FROM_BLOB(field, value) \
{ \
const auto bytes = (value).getRawBytes(); \
field = ByteArray(std::vector(bytes.begin(), bytes.end())); \
}

#define DB_CAST_TO_BLOB(field) \
mysqlx::bytes((field).Data(), (field).Size())
