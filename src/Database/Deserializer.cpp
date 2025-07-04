#include "Deserializer.h"

FDeserializer::~FDeserializer() {
    // for (const auto &val: resultMap_ | std::views::values) {
    //     delete val;
    // }
}

FDeserializer::FDeserializer(const std::shared_ptr<AResultMap> &result) {
    for (auto &[name, table]: *result)
        PushBack(name, std::move(table));
}

void FDeserializer::PushBack(const std::string &name, mysqlx::RowResult &&res) {
    auto ptr = std::make_unique<FEntityResult>(std::move(res));
    resultMap_.emplace(name, std::move(ptr));
}

FEntityResult *FDeserializer::FetchResult(const std::string &name) const {
    const auto it = resultMap_.find(name);
    return it == resultMap_.end() ? nullptr : it->second.get();
}
