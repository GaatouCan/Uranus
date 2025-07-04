#pragma once

#include <utility>

#include "Entity.h"

class BASE_API IEntityArray {

    std::string tableName_;

public:
    explicit IEntityArray(std::string tableName)
        : tableName_(std::move(tableName)) {
    }

    virtual ~IEntityArray() = default;

    DISABLE_COPY_MOVE(IEntityArray)

    void SetTableName(const std::string &name) { tableName_ = name; }
    [[nodiscard]] std::string GetTableName() const { return tableName_; }

    virtual void Serialize(mysqlx::Table &table) = 0;

    virtual void RemoveExpiredRow(mysqlx::Table &table, const std::string &expr) = 0;
};

template<CEntityType Type>
class BASE_API TEntityArray final : public IEntityArray {

    std::vector<Type> data_;

public:
    explicit TEntityArray(std::string tableName)
        : IEntityArray(std::move(tableName)) {
    }
    ~TEntityArray() override = default;

    void PushBack(const Type &value) {
        data_.push_back(value);
    }

    void PushBack(Type &&value) {
        data_.emplace_back(value);
    }

    template<class ... Args>
    void EmplaceBack(Args && ... args) {
        data_.emplace_back(std::forward<Args>(args)...);
    }

    void Serialize(mysqlx::Table &table) override {
        for (auto &value : data_) {
            value.Write(table);
        }
    }

    void RemoveExpiredRow(mysqlx::Table &table, const std::string &expr) override {
        auto res = table.select().where(expr).execute();

        if (res.count() == 0)
            return;

        std::vector<Type> del;
        bool bInclude = false;

        for (auto row: res) {
            // 如果将被写入的数据里面包含 则跳过
            for (auto &elem: data_) {
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
            Type elem;
            elem.Read(row);
            del.push_back(elem);
        }

        for (auto &row: del) {
            row.Remove(table);
        }
    }
};
