#include "../../../include/system/database/serializer.h"

Serializer::Serializer() {

}

Serializer::~Serializer() {
    for (const auto& [val, expired] : mTableVec) {
        delete val;
    }
}

void Serializer::Serialize(mysqlx::Schema &schema) {
    for (const auto &[val, expired] : mTableVec) {
        if (val == nullptr)
            continue;

        if (auto table = schema.getTable(val->GetTableName()); table.existsInDatabase()) {
            if (!expired.empty()) {
                val->DeleteExpiredRow(table, expired);
            }
            val->SerializeInternal(table);
        }
    }
}

