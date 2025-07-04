#pragma once

#include "EntityArray.h"

#include <memory>

class BASE_API FSerializer final {

    struct FTableNode {
        std::unique_ptr<IEntityArray> array;
        std::string expired_expr; // 清理过期数据的where语句
    };

    std::vector<FTableNode> tableList_;

public:
    FSerializer();
    ~FSerializer();

    DISABLE_COPY_MOVE(FSerializer)

    template<CEntityType T>
    TEntityArray<T> *AddEntityArray(const std::string &name, const std::string &expired = {}) {
        auto res = new TEntityArray<T>(name);

        auto ptr = std::unique_ptr<IEntityArray>(res);
        tableList_.emplace_back(std::move(ptr), expired);

        return res;
    }

    void Serialize(mysqlx::Schema &schema);
};
