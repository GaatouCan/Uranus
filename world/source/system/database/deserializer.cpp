#pragma once

#include "../../../include/system/database/deserializer.h"

Deserializer::Deserializer() {
}

Deserializer::~Deserializer() {
}

Deserializer::Deserializer(std::unordered_map<std::string, mysqlx::RowResult>&& result)
    : mResultMap(std::move(result)) {

}

void Deserializer::PushBack(const std::string& name, mysqlx::RowResult&& res) {
    mResultMap.insert_or_assign(name, std::move(res));
}

std::optional<TableResult> Deserializer::FetchResult(const std::string& name) {
    const auto it = mResultMap.find(name);
    if (it == mResultMap.end())
        return std::nullopt;

    return TableResult(std::move(it->second));
}
