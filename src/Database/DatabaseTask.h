#pragma once

#include "../common.h"

#include <functional>
#include <string>
#include <utility>
#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>
#include <mysqlx/xdevapi.h>


using AQueryArray           = std::vector<std::pair<std::string, std::string> >; // [表名, where表达式]
using AQueryResult          = absl::flat_hash_map<std::string, mysqlx::RowResult>; // [表名, 查询结果]
using AQueryResultPointer   = std::shared_ptr<AQueryResult>;

using ATransaction = std::function<void(mysqlx::Schema &)>;


class BASE_API IDBTask {

public:
    IDBTask() = default;
    virtual ~IDBTask() = default;

    DISABLE_COPY_MOVE(IDBTask)

    virtual void Execute(mysqlx::Schema &) noexcept = 0;
};

class BASE_API UDBTrans final : public IDBTask {

    ATransaction functor_;

public:
    explicit UDBTrans(ATransaction functor);

    void Execute(mysqlx::Schema &schema) noexcept override;
};


template<class Callable>
class TDBQuery final : public IDBTask {

    AQueryArray array_;
    Callable cb_;

public:
    TDBQuery(AQueryArray query, Callable &&cb)
        : array_(std::move(query)),
          cb_(std::forward<Callable>(cb)) {
    }

    void Execute(mysqlx::Schema &schema) noexcept override {
        auto ret = std::make_shared<AQueryResult>();
        try {
            for (const auto &[name, expr]: array_) {
                auto table = schema.getTable(name);
                if (!table.existsInDatabase()) {
                    SPDLOG_CRITICAL("{} - Table[{}] Does Not Exist.", __FUNCTION__, name);
                    continue;
                }
                ret->insert_or_assign(name, expr.empty() ? table.select().execute() : table.select().where(expr).execute());
                std::invoke(cb_, ret);
            }
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{} - Database Error: {}", __FUNCTION__, e.what());
            std::invoke(cb_, nullptr);
        }
    }
};
