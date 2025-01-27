#pragma once

#include <mysqlx/xdevapi.h>
#include <utility>
#include <vector>

using ADBQueryArray = std::vector<std::pair<std::string, std::string>>;
using ADBQueryResult = std::shared_ptr<std::unordered_map<std::string, mysqlx::RowResult>>;

using ADBTask = std::function<void(mysqlx::Schema &)>;

class IDatabaseWrapper {
public:
    virtual ~IDatabaseWrapper() = default;
    virtual void Execute(mysqlx::Schema &) = 0;
};


class UDBTaskWrapper final : public IDatabaseWrapper {

    ADBTask mTask;

public:
    UDBTaskWrapper() = delete;
    explicit UDBTaskWrapper(ADBTask task) : mTask(std::move(task)) {}

    void Execute(mysqlx::Schema &schema) override {
        std::invoke(mTask, schema);
    }
};


template<typename Callback>
class TDBQueryWrapper final : public IDatabaseWrapper {

    ADBQueryArray mArray;
    Callback mCallback;

public:
    TDBQueryWrapper() = delete;

    TDBQueryWrapper(ADBQueryArray array, Callback &&cb)
        : mArray(std::move(array)),
          mCallback(std::forward<Callback>(cb)) {
    }

    void Execute(mysqlx::Schema &schema) override {
        auto ret = std::make_shared<std::unordered_map<std::string, mysqlx::RowResult>>();
        for (const auto &[name,expr] : mArray) {
            if (auto table = schema.getTable(name); table.existsInDatabase()) {
                ret->insert_or_assign(name, table.select().where(expr).execute());
            }
        }
        std::invoke(mCallback, ret);
    }
};
