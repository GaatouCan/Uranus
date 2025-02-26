#include "../../../include/system/database/serializer.h"

Serializer::Serializer() {

}

Serializer::~Serializer() {
    for (const auto& [val, expired] : mVector) {
        delete val;
    }
}

void Serializer::Serialize(mysqlx::Schema &schema) {
    for (const auto &[val, expired] : mVector) {
        if (val == nullptr)
            continue;

        if (auto table = schema.getTable(val->GetTableName()); table.existsInDatabase()) {
            if (!expired.empty()) {
                val->RemoveExpiredRow(table, expired);
            }
            val->SerializeInternal(table);
        }
    }
}

