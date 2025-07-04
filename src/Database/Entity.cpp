#include "Entity.h"

mysqlx::RowResult IEntity::Query(mysqlx::Schema &schema) {
    if (auto table = schema.getTable(GetTableName()); table.existsInDatabase()) {
        return this->Query(table);
    }
    return {};
}

void IEntity::Write(mysqlx::Schema &schema) {
    if (auto table = schema.getTable(GetTableName()); table.existsInDatabase()) {
        this->Write(table);
    }
}

void IEntity::Remove(mysqlx::Schema &schema) {
    if (auto table = schema.getTable(GetTableName()); table.existsInDatabase()) {
        this->Remove(table);
    }
}
