#pragma once

#include "table_array.h"

class BASE_API USerializer final {

    struct FTableNode {
        ITableArray *array = nullptr;
        std::string expired_expr; // 清理过期数据的where语句
    };

    std::vector<FTableNode> table_vec_;

public:
    USerializer();
    ~USerializer();

    DISABLE_COPY_MOVE(USerializer)

    template<DB_TABLE_TYPE T>
    TTableArray<T> *CreateTableVector(const std::string &name, const std::string &expired = {}) {
        auto res = new TTableArray<T>(name);
        table_vec_.emplace_back(res, expired);
        return res;
    }

    void Serialize(mysqlx::Schema &schema);
};
