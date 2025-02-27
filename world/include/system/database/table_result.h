#pragma once

#include "db_table.h"

class BASE_API TableResult final {

    mysqlx::RowResult result_;
    mysqlx::Row cur_row_;

    const size_t total_;

public:
    explicit TableResult(mysqlx::RowResult &&res);

    DISABLE_COPY_MOVE(TableResult)

    [[nodiscard]] size_t TotalRowsCount() const;

    size_t Count();

    bool HasMore();

    template<DBTableType T>
    T DeserializeT() {
        T res;
        Deserialize(&res);
        return res;
    }

    void Deserialize(IDBTable *table);
};
