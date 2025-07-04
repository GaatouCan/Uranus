#include "Serializer.h"

FSerializer::FSerializer() {
}

FSerializer::~FSerializer() {
    // for (const auto &[val, expired]: tableList_) {
    //     delete val;
    // }
}

void FSerializer::Serialize(mysqlx::Schema &schema) {
    for (const auto &[val, expired]: tableList_) {
        if (val == nullptr)
            continue;

        if (auto table = schema.getTable(val->GetTableName()); table.existsInDatabase()) {
            if (!expired.empty()) {
                val->RemoveExpiredRow(table, expired);
            }
            val->Serialize(table);
        }
    }
}
