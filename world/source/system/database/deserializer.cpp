#include "../../../include/system/database/deserializer.h"

#include <ranges>

Deserializer::~Deserializer() {
    for (const auto& val : result_map_ | std::views::values) {
        delete val;
    }
}

Deserializer::Deserializer(const std::shared_ptr<std::unordered_map<std::string, mysqlx::RowResult>>& result) {
    for (auto& [name, table] : *result)
        PushBack(name, std::move(table));
}

void Deserializer::PushBack(const std::string& name, mysqlx::RowResult&& res) {
    result_map_[name] = new TableResult(std::move(res));
}

TableResult* Deserializer::FetchResult(const std::string& name) const {
    const auto it = result_map_.find(name);
    if (it == result_map_.end())
        return nullptr;

    return it->second;
}
