#include "../../../include/system/database/table_result.h"


UTableResult::UTableResult(mysqlx::RowResult &&res)
    : result_(std::move(res)),
      total_(result_.count()) {
}

size_t UTableResult::TotalRowsCount() const {
    return total_;
}

size_t UTableResult::Count() {
    return result_.count();
}

bool UTableResult::HasMore() {
    cur_row_ = result_.fetchOne();
    return !cur_row_.isNull();
}

void UTableResult::Deserialize(IDBTable* table) {
    if (cur_row_.isNull())
        cur_row_ = result_.fetchOne();

    if (cur_row_.isNull())
        return;

    table->Read(cur_row_);
}
