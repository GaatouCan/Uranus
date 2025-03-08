#pragma once

#include <utility>

#include "table.h"

class BASE_API ITableArray {

    std::string tableName_;

public:
    explicit ITableArray(std::string name)
        : tableName_(std::move(name)) {
    }

    virtual ~ITableArray() = default;

    DISABLE_COPY_MOVE(ITableArray)

    void setTableName(const std::string &name) { tableName_ = name; }
    [[nodiscard]] std::string getTableName() const { return tableName_; }

protected:
    friend class USerializer;

    virtual void serializeInternal(mysqlx::Table &table) = 0;
    virtual void removeExpiredRow(mysqlx::Table &table, const std::string &expr) = 0;
};

template<CTableType T>
class BASE_API TTableArray final : public ITableArray {

    std::vector<T> data_;

public:
    explicit TTableArray(std::string name)
        : ITableArray(std::move(name)) {
    }

    void pushBack(const T &value) {
        data_.push_back(value);
    }

    void pushBack(T &&value) {
        data_.emplace_back(value);
    }

    template<typename... Args>
    void emplaceBack(Args &&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
    }

private:
    friend class USerializer;

    void serializeInternal(mysqlx::Table &table) override {
        for (auto &value: data_) {
            value.write(table);
        }
    }

    void removeExpiredRow(mysqlx::Table &table, const std::string &expr) override {
        auto res = table.select().where(expr).execute();

        if (res.count() == 0)
            return;

        std::vector<T> del;
        bool bInclude = false;

        for (auto row: res) {
            // 如果将被写入的数据里面包含 则跳过
            for (auto &elem: data_) {
                if (elem.comparePrimaryKey(row)) {
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
            elem.read(row);
            del.push_back(elem);
        }

        for (auto &row: del) {
            row.remove(table);
        }
    }
};
