#pragma once

#include "table_array.h"

class BASE_API USerializer final {

    struct FTableNode {
        ITableArray *array = nullptr;
        std::string expired_expr; // 清理过期数据的where语句
    };

    std::vector<FTableNode> tables_;

public:
    USerializer();
    ~USerializer();

    DISABLE_COPY_MOVE(USerializer)

    template<CTableType T>
    TTableArray<T> *createTableVector(const std::string &name, const std::string &expired = {}) {
        auto res = new TTableArray<T>(name);
        tables_.emplace_back(res, expired);
        return res;
    }

    void serialize(mysqlx::Schema &schema);
};
