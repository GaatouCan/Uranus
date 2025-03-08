#pragma once

#include "table.h"

class BASE_API UTableResult final {

    mysqlx::RowResult result_;
    mysqlx::Row curRow_;

    const size_t total_;

public:
    explicit UTableResult(mysqlx::RowResult &&res);

    DISABLE_COPY_MOVE(UTableResult)

    [[nodiscard]] size_t totalRowsCount() const;

    size_t count();

    bool hasMore();

    template<CTableType T>
    T deserializeT() {
        T res;
        deserialize(&res);
        return res;
    }

    void deserialize(ITable *table);
};
