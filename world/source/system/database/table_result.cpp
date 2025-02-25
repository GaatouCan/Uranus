#include "../../../include/system/database/table_result.h"


TableResult::TableResult(mysqlx::RowResult &&res)
    : mResult(std::move(res)),
      mTotal(mResult.count()) {
}

size_t TableResult::TotalRowsCount() const {
    return mTotal;
}

size_t TableResult::Count() {
    return mResult.count();
}

bool TableResult::HasMore() {
    mCurrentRow = mResult.fetchOne();
    return !mCurrentRow.isNull();
}

void TableResult::Deserialize(IDBTable* table) {
    if (mCurrentRow.isNull())
        mCurrentRow = mResult.fetchOne();

    if (mCurrentRow.isNull())
        return;

    table->Read(mCurrentRow);
}
