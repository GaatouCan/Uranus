#pragma once

#include "db_table.h"

class BASE_API UTableResult final {

    mysqlx::RowResult result_;
    mysqlx::Row cur_row_;

    const size_t total_;

public:
    explicit UTableResult(mysqlx::RowResult &&res);

    DISABLE_COPY_MOVE(UTableResult)

    [[nodiscard]] size_t TotalRowsCount() const;

    size_t Count();

    bool HasMore();

    template<DB_TABLE_TYPE T>
    T DeserializeT() {
        T res;
        Deserialize(&res);
        return res;
    }

    void Deserialize(IDBTable *table);
};
