#pragma once

#include "table_result.h"

class BASE_API Deserializer final {

    std::unordered_map<std::string, mysqlx::RowResult> mResultMap;

public:
    Deserializer();
    ~Deserializer();

    DISABLE_COPY_MOVE(Deserializer)

    explicit Deserializer(std::unordered_map<std::string, mysqlx::RowResult> &&result);

    void PushBack(const std::string& name, mysqlx::RowResult && res);

    std::optional<TableResult> FetchResult(const std::string &name);
};
