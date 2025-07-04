#include "DataAccess.h"
#include "../Server.h"

#include <spdlog/spdlog.h>

UDataAccess::UDataAccess(UServer *server)
    : Super(server),
      nextNodeIndex_(0) {
}

void UDataAccess::Initial() {
    if (mState != EModuleState::CREATED)
        return;

    sessionList_ = std::vector<FSessionNode>(1);

    auto host = "localhost";
    auto port = 33060;
    auto user = "root";
    auto passwd = "12345678";

    for (auto &node : sessionList_) {
        node.session = std::make_unique<mysqlx::Session>(host, port, user, passwd);
        node.queue = std::make_unique<ADBTaskQueue>();
        node.thread = std::make_unique<std::thread>([this, &node]() {
            while (node.queue->IsRunning() && mState == EModuleState::RUNNING) {
                node.queue->Wait();
                if (!node.queue->IsRunning() || mState != EModuleState::RUNNING)
                    break;

                try {
                    const auto task = std::move(node.queue->PopFront());
                    auto schema = node.session->getSchema("demo", true);
                    task->Execute(schema);
                } catch (const std::exception &e) {
                    SPDLOG_ERROR("Database Task Error: {}", e.what());
                }
            }

            node.queue->Clear();
            node.session->close();
        });
    }

    mState = EModuleState::INITIALIZED;
}

void UDataAccess::Stop() {
    if (mState == EModuleState::STOPPED)
        return;

    mState = EModuleState::STOPPED;

    for (const auto &[th, sess, queue]: sessionList_) {
        if (queue)
            queue->Quit();
    }
}

UDataAccess::~UDataAccess() {
    for (auto &[th, sess, queue]: sessionList_) {
        if (th->joinable())
            th->join();
    }
}

void UDataAccess::PushTransaction(const ATransaction &tans) {
    if (mState != EModuleState::RUNNING)
        return;

    if (sessionList_.empty()) {
        GetServer()->Shutdown();
        exit(-7);
    }

    const auto &[th, sess, queue] = sessionList_[nextNodeIndex_++];
    nextNodeIndex_ = nextNodeIndex_ % sessionList_.size();
    auto node = std::make_unique<UDBTrans>(tans);
    queue->PushBack(std::move(node));
}

void UDataAccess::SyncSelect(const std::string &tableName, const std::string &where, const std::function<void(mysqlx::Row)> &cb) {
    const auto &[th, sess, queue] = sessionList_.front();

    auto schema = sess->getSchema("demo");
    if (!schema.existsInDatabase())
        return;

    auto table = schema.getTable("demo");
    if (!table.existsInDatabase())
        return;

    if (where.empty()) {
        for (auto result = table.select().execute(); const auto &row: result) {
            cb(row);
        }
    } else {
        for (auto result = table.select().where(where).execute(); const auto &row: result) {
            cb(row);
        }
    }
}
