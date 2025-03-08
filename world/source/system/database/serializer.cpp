#include "../../../include/system/database/serializer.h"

USerializer::USerializer() {

}

USerializer::~USerializer() {
    for (const auto& [val, expired] : tables_) {
        delete val;
    }
}

void USerializer::serialize(mysqlx::Schema &schema) {
    for (const auto &[val, expired] : tables_) {
        if (val == nullptr)
            continue;

        if (auto table = schema.getTable(val->getTableName()); table.existsInDatabase()) {
            if (!expired.empty()) {
                val->removeExpiredRow(table, expired);
            }
            val->serializeInternal(table);
        }
    }
}

