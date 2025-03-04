#pragma once

#include "table_array.h"

class BASE_API Serializer final {

    struct TableNode {
        ITableArray *array = nullptr;
        std::string expired_expr; // 清理过期数据的where语句
    };

    std::vector<TableNode> table_vec_;

public:
    Serializer();
    ~Serializer();

    DISABLE_COPY_MOVE(Serializer)

    template<DBTableType T>
    TableArray<T> *CreateTableVector(const std::string &name, const std::string &expired = {}) {
        auto res = new TableArray<T>(name);
        table_vec_.emplace_back(res, expired);
        return res;
    }

    void Serialize(mysqlx::Schema &schema);
};
