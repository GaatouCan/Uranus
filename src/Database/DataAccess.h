#pragma once

#include "../Module.h"
#include "../ConcurrentDeque.h"
#include "DatabaseTask.h"
#include "../Utils.h"

#include <memory>
#include <thread>


class BASE_API UDataAccess final : public IModule {

    DECLARE_MODULE(UDataAccess)

    using ADBTaskQueue = TConcurrentDeque<std::unique_ptr<IDBTask>>;

    struct FSessionNode {
        std::unique_ptr<std::thread> thread;
        std::unique_ptr<mysqlx::Session> session;
        std::unique_ptr<ADBTaskQueue> queue;
    };

protected:
    explicit UDataAccess(UServer *server);

    void Initial() override;
    void Stop() override;

public:
    ~UDataAccess() override;

    constexpr const char *GetModuleName() const override {
        return "Data Access";
    }

    void PushTransaction(const ATransaction &tans);

    void SyncSelect(const std::string &tableName, const std::string &where, const std::function<void(mysqlx::Row)> &cb);

    template<typename Callable>
    void PushQuery(const AQueryArray &vec, Callable &&cb) {
        if (mState != EModuleState::RUNNING)
            return;

        if (sessionList_.empty())
            return;

        const auto &[th, sess, queue] = sessionList_[nextNodeIndex_++];
        nextNodeIndex_ = nextNodeIndex_ % sessionList_.size();
        auto node = std::make_unique<TDBQuery<Callable>>(vec, std::forward<Callable>(cb));
        queue->PushBack(std::move(node));
    }

    template<asio::completion_token_for<void(AQueryResultPointer)> CompletionToken>
    auto AsyncSelect(const AQueryArray &vec, CompletionToken &&token) {
        auto init = [this](asio::completion_handler_for<void(AQueryResultPointer)> auto handle, const AQueryArray &query) {
            auto work = asio::make_work_guard(handle);

            if (mState != EModuleState::RUNNING) {
                auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
                asio::dispatch(work.get_executor(), asio::bind_allocator(alloc, [handle = std::move(handle)]() mutable {
                    std::move(handle)(nullptr);
                }));
                return;
            }

            this->PushQuery(query, [handle = std::move(handle), work = std::move(work)](AQueryResultPointer result) mutable {
                auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
                asio::dispatch(work.get_executor(), asio::bind_allocator(alloc, [handle = std::move(handle), result = std::forward<AQueryResultPointer>(result)]() mutable {
                    std::move(handle)(std::move(result));
                }));
            });
        };

        return asio::async_initiate<CompletionToken, void(AQueryResultPointer)>(init, token, vec);
    };

private:
    std::vector<FSessionNode> sessionList_;
    std::atomic_size_t nextNodeIndex_;
};

