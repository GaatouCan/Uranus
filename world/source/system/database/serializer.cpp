#include "../../../include/system/database/serializer.h"

Serializer::Serializer() {

}

Serializer::~Serializer() {
    for (const auto& [val, expired] : table_vec_) {
        delete val;
    }
}

void Serializer::Serialize(mysqlx::Schema &schema) {
    for (const auto &[val, expired] : table_vec_) {
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

