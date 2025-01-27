#pragma once

#include "DBTable.h"

#include <mysqlx/xdevapi.h>

class UDeserializer final {

    mysqlx::RowResult mResult;
    mysqlx::Row mCurRow;

public:
    explicit UDeserializer(mysqlx::RowResult res)
        : mResult(std::move(res)) {
    }

    DISABLE_COPY_MOVE(UDeserializer)

    bool HasMore() {
        mCurRow = mResult.fetchOne();
        return !mCurRow.isNull();
    }

    template<DB_TABLE_TYPE T>
    T DeserializeT() {
        T res;
        Deserialize(&res);
        return res;
    }

    void Deserialize(IDBTable *row) {
        if (mCurRow.isNull())
            mCurRow = mResult.fetchOne();

        if (mCurRow.isNull())
            return;

        row->Read(mCurRow);
    }
};
