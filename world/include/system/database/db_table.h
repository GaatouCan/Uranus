#pragma once

#include "../../common.h"

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
