#pragma once

#include "Entity.h"

class BASE_API FEntityResult final {

    mysqlx::RowResult result_;
    mysqlx::Row curRow_;

    const size_t total_;

public:
    FEntityResult() = delete;

    explicit FEntityResult(mysqlx::RowResult &&res);

    DISABLE_COPY_MOVE(FEntityResult)

    [[nodiscard]] size_t TotalRowsCount() const;

    size_t Count();

    bool HasMore();

    template<CEntityType T>
    T DeserializeT() {
        T res;
        this->Deserialize(&res);
        return res;
    }

    void Deserialize(IEntity *entity);
};

