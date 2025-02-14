#pragma once

#include "db_table.h"

class BASE_API Deserializer final {

    mysqlx::RowResult mResult;
    mysqlx::Row mCurrentRow;

    const size_t mTotal;

public:
    explicit Deserializer(mysqlx::RowResult res)
        : mResult(std::move(res)),
          mTotal(mResult.count()) {

    }

    DISABLE_COPY_MOVE(Deserializer)

    [[nodiscard]] size_t TotalRowsCount() const {
        return mTotal;
    }

    size_t Count() {
        return mResult.count();
    }

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
