#pragma once

#include "../../sub_system.h"
#include "../../thread_safe_deque.h"
#include "../../utils.h"
#include "database_task.h"

#include <memory>
#include <thread>
#include <mysqlx/xdevapi.h>


class BASE_API DatabaseSystem final : public ISubSystem {

    struct SessionNode {
        std::unique_ptr<std::thread> thread;
        std::unique_ptr<mysqlx::Session> session;
        std::unique_ptr<ThreadSafeDeque<IDatabaseTask *>> queue;
        ThreadID threadID;
    };

    std::vector<SessionNode> sess_list_;
    std::atomic_size_t next_node_idx_ = 0;

public:
    explicit DatabaseSystem(GameWorld *world);
    ~DatabaseSystem() override;

    GET_SYSTEM_NAME(DatabaseSystem)

    void Init() override;

    void SyncSelect(const std::string &tableName, const std::string &where, const std::function<void(mysqlx::Row)> &cb) const;

    void PushTransaction(const TransactionFunctor &func);

    template<class Callback>
    void PushSelectTask(const QueryArray &vec, Callback &&cb) {
        const auto &[th, sess, queue, tid] = sess_list_[next_node_idx_++];
        next_node_idx_ = next_node_idx_ % sess_list_.size();
        queue->PushBack(new QueryTask<Callback>(vec, std::forward<Callback>(cb)));
    }

    template<asio::completion_token_for<void(QueryResultPtr)> CompletionToken>
    auto AsyncSelect(const QueryArray &vec, CompletionToken &&token) {
        auto init = [this](asio::completion_handler_for<void(QueryResultPtr)> auto handle, const QueryArray &query) {
            auto work = asio::make_work_guard(handle);

            PushSelectTask(query, [handle = std::move(handle), work = std::move(work)](QueryResultPtr result) mutable {
                auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
                asio::dispatch(work.get_executor(), asio::bind_allocator(alloc, [handle = std::move(handle), result = std::forward<QueryResultPtr>(result)]() mutable {
                    std::move(handle)(std::move(result));
                }));
            });
        };

        return asio::async_initiate<CompletionToken, void(QueryResultPtr)>(init, token, vec);
    };
};
