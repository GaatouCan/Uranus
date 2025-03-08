#pragma once

#include "../../sub_system.h"
#include "../../ts_deque.h"
#include "../../utils.h"
#include "database_task.h"

#include <memory>
#include <thread>
#include <mysqlx/xdevapi.h>


class BASE_API UDatabaseSystem final : public ISubSystem {

    struct FSessionNode {
        std::unique_ptr<std::thread> thread;
        std::unique_ptr<mysqlx::Session> session;
        std::unique_ptr<TDeque<IDatabaseTask *>> queue;
        AThreadID threadID;
    };

    std::vector<FSessionNode> sessionList_;
    std::atomic_size_t nextNodeIndex_ = 0;

public:
    explicit UDatabaseSystem(UGameWorld *world);
    ~UDatabaseSystem() override;

    GET_SYSTEM_NAME(DatabaseSystem)

    void init() override;

    void syncSelect(const std::string &tableName, const std::string &where, const std::function<void(mysqlx::Row)> &cb) const;

    void pushTransaction(const ATransactionFunctor &func);

    template<class Callback>
    void pushSelectTask(const AQueryArray &vec, Callback &&cb) {
        const auto &[th, sess, queue, tid] = sessionList_[nextNodeIndex_++];
        nextNodeIndex_ = nextNodeIndex_ % sessionList_.size();
        queue->pushBack(new UQueryTask<Callback>(vec, std::forward<Callback>(cb)));
    }

    template<asio::completion_token_for<void(AQueryResultPtr)> CompletionToken>
    auto asyncSelect(const AQueryArray &vec, CompletionToken &&token) {
        auto init = [this](asio::completion_handler_for<void(AQueryResultPtr)> auto handle, const AQueryArray &query) {
            auto work = asio::make_work_guard(handle);

            pushSelectTask(query, [handle = std::move(handle), work = std::move(work)](AQueryResultPtr result) mutable {
                auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
                asio::dispatch(work.get_executor(), asio::bind_allocator(alloc, [handle = std::move(handle), result = std::forward<AQueryResultPtr>(result)]() mutable {
                    std::move(handle)(std::move(result));
                }));
            });
        };

        return asio::async_initiate<CompletionToken, void(AQueryResultPtr)>(init, token, vec);
    };
};
