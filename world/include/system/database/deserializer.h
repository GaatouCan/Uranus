#pragma once

#include "table_result.h"

class BASE_API Deserializer final {

    std::unordered_map<std::string, TableResult *> mResultMap;

public:
    Deserializer();
    ~Deserializer();

    DISABLE_COPY_MOVE(Deserializer)

    explicit Deserializer(const std::shared_ptr<std::unordered_map<std::string, mysqlx::RowResult>> &result);

    void PushBack(const std::string& name, mysqlx::RowResult && res);

    [[nodiscard]] TableResult* FetchResult(const std::string &name) const;
};
