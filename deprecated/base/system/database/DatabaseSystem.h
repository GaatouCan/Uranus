#pragma once

#include "../../SubSystem.h"
#include "../../TSDeque.h"
#include "DatabaseWrapper.h"

#include <thread>
#include <vector>
#include <utility>


struct FDatabaseNode {
    std::unique_ptr<std::thread> th;
    std::unique_ptr<mysqlx::Session> sess;
    std::unique_ptr<TSDeque<IDatabaseWrapper *> > queue;
    AThreadID tid;
};

class UDatabaseSystem final : public ISubSystem {

    std::vector<FDatabaseNode> mNodeList;
    std::atomic_size_t mNextNodeIndex = 0;

public:
    UDatabaseSystem();
    ~UDatabaseSystem() override;

    SUB_SYSTEM_BODY(UDatabaseSystem)

    void Init() override;

    /**
     * 阻塞SQL查询，只能用于服务初始化时数据加载
     * @param tableName 表名
     * @param where 条件语句
     * @param cb 回调函数
     */
    void BlockSelect(const std::string &tableName, const std::string &where, const std::function<void(mysqlx::Row)> &cb);

    void PushTask(const ADBTask &task);

    template<typename Callback>
    void PushSelectTask(const ADBQueryArray &vec, Callback &&cb) {
        const auto &[th, sess, queue, tid] = mNodeList[mNextNodeIndex++];
        mNextNodeIndex = mNextNodeIndex % mNodeList.size();

        queue->PushBack(new TDBQueryWrapper<Callback>(vec, std::forward<Callback>(cb)));
    }


    template<asio::completion_token_for<void(ADBQueryResult)> CompletionToken>
    auto AsyncSelect(const ADBQueryArray &vec, CompletionToken &&token) {
        auto init = [this](asio::completion_handler_for<void(std::shared_ptr<std::vector<mysqlx::RowResult>>)> auto handler, const ADBQueryArray &query) {
            auto work = asio::make_work_guard(handler);

            PushSelectTask(query, [handler = std::move(handler), work = std::move(work)](ADBQueryResult result) mutable {
                auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());
                asio::dispatch(work.get_executor(), asio::bind_allocator(alloc, [handler = std::move(handler), result = std::forward<ADBQueryResult>(result)]() mutable {
                    std::move(handler)(std::move(result));
                }));
            });
        };

        return asio::async_initiate<CompletionToken, void(ADBQueryResult)>(init, token, vec);
    }
};
