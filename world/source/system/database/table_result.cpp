#include "../../../include/system/database/table_result.h"


TableResult::TableResult(mysqlx::RowResult &&res)
    : result_(std::move(res)),
      total_(result_.count()) {
}

size_t TableResult::TotalRowsCount() const {
    return total_;
}

size_t TableResult::Count() {
    return result_.count();
}

bool TableResult::HasMore() {
    cur_row_ = result_.fetchOne();
    return !cur_row_.isNull();
}

void TableResult::Deserialize(IDBTable* table) {
    if (cur_row_.isNull())
        cur_row_ = result_.fetchOne();

    if (cur_row_.isNull())
        return;

    table->Read(cur_row_);
}
