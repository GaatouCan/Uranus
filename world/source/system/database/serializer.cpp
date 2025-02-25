#include "../../../include/system/database/serializer.h"

Serializer::Serializer() {

}

Serializer::~Serializer() {

}

void Serializer::Serialize(mysqlx::Schema &schema) {
    for (const auto &[val, expired] : mVector) {
        if (val == nullptr)
            continue;

        if (auto table = schema.getTable(val->GetTableName()); table.existsInDatabase()) {
            if (!expired.empty()) {
                val->RemoveExpiredData(table, expired);
            }
            val->Serialize(table);
        }
        delete val;
    }
}

