#pragma once

#include "../../common.h"
#include "../../byte_array.h"

#include <mysqlx/xdevapi.h>


class BASE_API ITable {

public:
    virtual ~ITable() = default;

    [[nodiscard]] virtual const char* getTableName() const = 0;

    [[nodiscard]] virtual bool comparePrimaryKey(mysqlx::Row &row) const = 0;

    virtual mysqlx::RowResult query(mysqlx::Table &table) = 0;
    virtual mysqlx::RowResult query(mysqlx::Schema &schema) = 0;

    virtual void read(mysqlx::Row &row) = 0;

    virtual void write(mysqlx::Table &table) = 0;
    virtual void write(mysqlx::Schema &schema) = 0;

    virtual void remove(mysqlx::Table &table) = 0;
    virtual void remove(mysqlx::Schema &schema) = 0;
};

template <typename T>
concept CTableType = std::derived_from<T, ITable>;

#define DB_CAST_FROM_BLOB(field, value) \
{ \
const auto bytes = (value).getRawBytes(); \
field = FByteArray(std::vector(bytes.begin(), bytes.end())); \
}

#define DB_CAST_TO_BLOB(field) \
mysqlx::bytes((field).Data(), (field).Size())
