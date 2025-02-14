#pragma once

#include "db_table.h"

class BASE_API Deserializer final {

    mysqlx::RowResult mResult;
    mysqlx::Row mCurrentRow;

public:
    explicit Deserializer(mysqlx::RowResult res)
        : mResult(std::move(res)) {

    }

    DISABLE_COPY_MOVE(Deserializer)

    bool HasMore() {
        mCurrentRow = mResult.fetchOne();
        return !mCurrentRow.isNull();
    }

    template<DBTableType T>
    T DeserializeT() {
        T res;
        Deserialize(&res);
        return res;
    }

    void Deserialize(IDBTable *table) {
        if (mCurrentRow.isNull())
            mCurrentRow = mResult.fetchOne();

        if (mCurrentRow.isNull())
            return;

        table->Read(mCurrentRow);
    }
};
