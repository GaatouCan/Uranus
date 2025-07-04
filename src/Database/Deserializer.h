#pragma once

#include "EntityResult.h"

#include <memory>
#include <absl/container/flat_hash_map.h>

class BASE_API FDeserializer final {

    using AResultMap = absl::flat_hash_map<std::string, mysqlx::RowResult>;

    absl::flat_hash_map<std::string, std::unique_ptr<FEntityResult>> resultMap_;

public:
    FDeserializer() = default;
    ~FDeserializer();

    DISABLE_COPY_MOVE(FDeserializer)

    explicit FDeserializer(const std::shared_ptr<AResultMap> &result);

    void PushBack(const std::string &name, mysqlx::RowResult &&res);
    [[nodiscard]] FEntityResult *FetchResult(const std::string &name) const;
};
