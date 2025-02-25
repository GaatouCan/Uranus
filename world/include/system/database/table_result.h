#pragma once

#include "db_table.h"

class BASE_API TableResult final {

    mysqlx::RowResult mResult;
    mysqlx::Row mCurrentRow;

    const size_t mTotal;

public:
    explicit TableResult(mysqlx::RowResult res);

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
