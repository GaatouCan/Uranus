#pragma once

#include <utility>

#include "db_table.h"

class BASE_API ISerializer {
    std::string mTableName;

public:
    explicit ISerializer(std::string name)
        : mTableName(std::move(name)) {
    }

    virtual ~ISerializer() = default;

    void SetTableName(const std::string &name) { mTableName = name; }
    [[nodiscard]] std::string GetTableName() const { return mTableName; }

    virtual void Serialize(mysqlx::Table &table) = 0;

    virtual void RemoveExpiredData(mysqlx::Table &table, const std::string &expr) = 0;
};

template<DBTableType T>
class BASE_API Serializer final : public ISerializer {
    std::vector<T> mData;

public:
    explicit Serializer(std::string name)
        : ISerializer(std::move(name)) {
    }

    void PushBack(const T &value) {
        mData.push_back(value);
    }

    void PushBack(T &&value) {
        mData.emplace_back(value);
    }

    template<typename... Args>
    void EmplaceBack(Args &&... args) {
        mData.emplace_back(std::forward<Args>(args)...);
    }

    void Serialize(mysqlx::Table &table) override {
        for (auto &value: mData) {
            value.Write(table);
        }
    }

    void RemoveExpiredData(mysqlx::Table &table, const std::string &expr) override {
        auto res = table.select().where(expr).execute();

        if (res.count() == 0)
            return;

        std::vector<T> del;
        bool bInclude = false;

        for (auto row: res) {
            // 如果将被写入的数据里面包含 则跳过
            for (auto &elem: mData) {
                if (elem.ComparePrimaryKey(row)) {
                    bInclude = true;
                    break;
                }
            }

            if (bInclude) {
                bInclude = false;
                continue;
            }

            // 添加到删除列表里
            T elem;
            elem.Read(row);
            del.push_back(elem);
        }

        for (auto &row: del) {
            row.Remove(table);
        }
    }
};
