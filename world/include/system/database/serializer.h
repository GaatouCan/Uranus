#pragma once

#include "table_vector.h"


class BASE_API Serializer final {
    struct TableNode {
        ITableVector *ptr = nullptr;
        std::string expired;
    };
    std::vector<TableNode> mVector;
public:
    Serializer();
    ~Serializer();

    template<DBTableType T>
    ITableVector *CreateTableVector(const std::string &name, const std::string &expired = {}) {
        auto res = new TableVector<T>(name);
        mVector.emplace_back(res, expired);
        return res;
    }

    void Serialize(mysqlx::Schema &schema);
};
