#pragma once

#include "DBTable.h"
#include "../../common.h"

#include <mysqlx/xdevapi.h>
#include <utility>

class ISerializer {

    std::string mTableName;

public:
    explicit ISerializer(const std::string_view name)
        : mTableName(name) {}

    virtual ~ISerializer() = default;

    [[nodiscard]] std::string GetTableName() const { return mTableName; }

    virtual void Execute(mysqlx::Table &table) = 0;
    virtual void RemoveExpiredData(mysqlx::Table &table, const std::string &expr) = 0;
};

template<DB_TABLE_TYPE T>
class USerializer final : public ISerializer {

    std::vector<T> mVector;

public:
    explicit USerializer(const std::string_view name)
        : ISerializer(name){}


    void PushBack(T && row) {
        mVector.emplace_back(row);
    }

    void PushBack(const T & row) {
        mVector.push_back(row);
    }

    template<typename ... Args>
    void EmplaceBack(Args &&... args) {
        mVector.emplace_back(std::forward<Args>(args)...);
    }

    void Execute(mysqlx::Table &table) override {
        for (auto &row : mVector) {
            row.Write(table);
        }
    }

    void RemoveExpiredData(mysqlx::Table &table, const std::string &expr) override {
        auto res = table.select().where(expr).execute();
        if (res.count() == 0)
            return;

        std::vector<T> del;
        for (auto row : res) {
            for (auto &elem : mVector) {
                if (elem.ComparePrimaryKey(row))
                    break;
            }
            T elem;
            elem.Read(row);
            del.push_back(elem);
        }

        for (auto &row : mVector) {
            row.Remove(table);
        }
    }
};
