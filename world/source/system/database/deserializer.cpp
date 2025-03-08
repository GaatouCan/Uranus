#include "../../../include/system/database/deserializer.h"

#include <ranges>

UDeserializer::~UDeserializer() {
    for (const auto& val : result_map_ | std::views::values) {
        delete val;
    }
}

UDeserializer::UDeserializer(const std::shared_ptr<std::unordered_map<std::string, mysqlx::RowResult>>& result) {
    for (auto& [name, table] : *result)
        pushBack(name, std::move(table));
}

void UDeserializer::pushBack(const std::string& name, mysqlx::RowResult&& res) {
    result_map_[name] = new UTableResult(std::move(res));
}

UTableResult* UDeserializer::fetchResult(const std::string& name) const {
    const auto it = result_map_.find(name);
    if (it == result_map_.end())
        return nullptr;

    return it->second;
}
