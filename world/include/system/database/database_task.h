#pragma once

#include "../../common.h"

#include <functional>
#include <string>
#include <mysqlx/xdevapi.h>

using QueryArray = std::vector<std::pair<std::string, std::string>>;        // [表名, where表达式]
using QueryResult = std::unordered_map<std::string, mysqlx::RowResult>;     // [表名, 查询结果]
using QueryResultPtr = std::shared_ptr<QueryResult>;

using TransactionFunctor = std::function<void(mysqlx::Schema &)>;

class BASE_API IDatabaseTask {
public:
    virtual ~IDatabaseTask() = default;
    virtual void Execute(mysqlx::Schema &) = 0;
};

class BASE_API TransactionTask final : public IDatabaseTask {

    TransactionFunctor mFunctor;

public:
    TransactionTask() = delete;

    explicit TransactionTask(TransactionFunctor functor)
        : mFunctor(std::move(functor)) {
    }

    void Execute(mysqlx::Schema &schema) override {
        schema.getSession().startTransaction();
        mFunctor(schema);
        schema.getSession().commit();
    }
};


template<class Callable>
class BASE_API QueryTask final : public IDatabaseTask {

    QueryArray mArray;
    Callable mCallback;

public:
    QueryTask() = delete;

    QueryTask(QueryArray query, Callable &&cb)
        : mArray(std::move(query)),
          mCallback(std::forward<Callable>(cb)) {
    }

    void Execute(mysqlx::Schema &schema) override {
        auto ret = std::make_shared<QueryResult>();
        for (const auto &[name, expr]: mArray) {
            auto table = schema.getTable(name, true);
            ret->insert_or_assign(name, expr.empty() ? table.select().execute() : table.select().where(expr).execute());
        }
        mCallback(ret);
    }
};
