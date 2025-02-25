#pragma once

#include "table_array.h"

class BASE_API Serializer final {
    struct TableNode {
        ITableArray *array = nullptr;
        std::string expiredExpress;
    };
    std::vector<TableNode> mTableVec;
public:
    Serializer();
    ~Serializer();

    DISABLE_COPY_MOVE(Serializer)

    template<DBTableType T>
    TableArray<T> *CreateTableVector(const std::string &name, const std::string &expiredExpr = {}) {
        auto res = new TableArray<T>(name);
        mTableVec.emplace_back(res, expiredExpr);
        return res;
    }

    void Serialize(mysqlx::Schema &schema);
};
