#pragma once

#include "table_result.h"

class BASE_API UDeserializer final {

    std::unordered_map<std::string, UTableResult *> result_map_;

public:
    UDeserializer() = default;
    ~UDeserializer();

    DISABLE_COPY_MOVE(UDeserializer)

    explicit UDeserializer(const std::shared_ptr<std::unordered_map<std::string, mysqlx::RowResult>> &result);

    void PushBack(const std::string& name, mysqlx::RowResult && res);

    [[nodiscard]] UTableResult* FetchResult(const std::string &name) const;
};
