#pragma once

#include <absl/container/flat_hash_map.h>

#include "table_result.h"

class BASE_API UDeserializer final {

    absl::flat_hash_map<std::string, UTableResult *> result_map_;

public:
    UDeserializer() = default;
    ~UDeserializer();

    DISABLE_COPY_MOVE(UDeserializer)

    explicit UDeserializer(const std::shared_ptr<absl::flat_hash_map<std::string, mysqlx::RowResult>> &result);

    void pushBack(const std::string& name, mysqlx::RowResult && res);

    [[nodiscard]] UTableResult* fetchResult(const std::string &name) const;
};
