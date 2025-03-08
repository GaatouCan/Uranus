#include "../../../include/system/database/table_result.h"


UTableResult::UTableResult(mysqlx::RowResult &&res)
    : result_(std::move(res)),
      total_(result_.count()) {
}

size_t UTableResult::TotalRowsCount() const {
    return total_;
}

size_t UTableResult::count() {
    return result_.count();
}

bool UTableResult::hasMore() {
    curRow_ = result_.fetchOne();
    return !curRow_.isNull();
}

void UTableResult::deserialize(ITable* table) {
    if (curRow_.isNull())
        curRow_ = result_.fetchOne();

    if (curRow_.isNull())
        return;

    table->read(curRow_);
}
