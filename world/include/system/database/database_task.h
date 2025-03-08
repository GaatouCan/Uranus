#pragma once

#include "../../common.h"

#include <functional>
#include <string>
#include <mysqlx/xdevapi.h>

using AQueryArray = std::vector<std::pair<std::string, std::string>>;        // [表名, where表达式]
using AQueryResult = std::unordered_map<std::string, mysqlx::RowResult>;     // [表名, 查询结果]
using AQueryResultPtr = std::shared_ptr<AQueryResult>;

using ATransactionFunctor = std::function<void(mysqlx::Schema &)>;

class BASE_API IDatabaseTask {
public:
    virtual ~IDatabaseTask() = default;
    virtual void execute(mysqlx::Schema &) = 0;
};

class BASE_API UTransactionTask final : public IDatabaseTask {

    ATransactionFunctor functor_;

public:
    UTransactionTask() = delete;

    explicit UTransactionTask(ATransactionFunctor functor)
        : functor_(std::move(functor)) {
    }

    void execute(mysqlx::Schema &schema) override {
        schema.getSession().startTransaction();
        functor_(schema);
        schema.getSession().commit();
    }
};


template<class Callable>
class BASE_API UQueryTask final : public IDatabaseTask {

    AQueryArray array_;
    Callable cb_;

public:
    UQueryTask() = delete;

    UQueryTask(AQueryArray query, Callable &&cb)
        : array_(std::move(query)),
          cb_(std::forward<Callable>(cb)) {
    }

    void execute(mysqlx::Schema &schema) override {
        auto ret = std::make_shared<AQueryResult>();
        for (const auto &[name, expr]: array_) {
            auto table = schema.getTable(name, true);
            ret->insert_or_assign(name, expr.empty() ? table.select().execute() : table.select().where(expr).execute());
        }
        cb_(ret);
    }
};
