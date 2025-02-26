#include "../../../include/system/database/deserializer.h"

#include <ranges>

Deserializer::~Deserializer() {
    for (const auto& val : mResultMap | std::views::values) {
        delete val;
    }
}

Deserializer::Deserializer(const std::shared_ptr<std::unordered_map<std::string, mysqlx::RowResult>>& result) {
    for (auto& [name, table] : *result)
        PushBack(name, std::move(table));
}

void Deserializer::PushBack(const std::string& name, mysqlx::RowResult&& res) {
    mResultMap[name] = new TableResult(std::move(res));
}

TableResult* Deserializer::FetchResult(const std::string& name) const {
    const auto it = mResultMap.find(name);
    if (it == mResultMap.end())
        return nullptr;

    return it->second;
}
