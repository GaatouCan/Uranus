#include "../../../include/system/database/deserializer.h"

#include <ranges>

UDeserializer::~UDeserializer() {
    for (const auto& val : result_map_ | std::views::values) {
        delete val;
    }
}

UDeserializer::UDeserializer(const std::shared_ptr<std::unordered_map<std::string, mysqlx::RowResult>>& result) {
    for (auto& [name, table] : *result)
        PushBack(name, std::move(table));
}

void UDeserializer::PushBack(const std::string& name, mysqlx::RowResult&& res) {
    result_map_[name] = new UTableResult(std::move(res));
}

UTableResult* UDeserializer::FetchResult(const std::string& name) const {
    const auto it = result_map_.find(name);
    if (it == result_map_.end())
        return nullptr;

    return it->second;
}
