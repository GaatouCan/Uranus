#include "EntityResult.h"

FEntityResult::FEntityResult(mysqlx::RowResult &&res)
    : result_(std::move(res)),
      total_(result_.count()) {
}

size_t FEntityResult::TotalRowsCount() const {
    return total_;
}

size_t FEntityResult::Count() {
    return result_.count();
}

bool FEntityResult::HasMore() {
    curRow_ = result_.fetchOne();
    return !curRow_.isNull();
}

void FEntityResult::Deserialize(IEntity *entity) {
    if (curRow_.isNull())
        curRow_ = result_.fetchOne();

    if (curRow_.isNull())
        return;

    entity->Read(curRow_);
}
