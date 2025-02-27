#include "../../../include/system/database/database_system.h"
#include "game_world.h"
#include "../../../include/config_manager.h"

#include <spdlog/spdlog.h>

DatabaseSystem::DatabaseSystem(GameWorld *world)
    : ISubSystem(world) {
}

DatabaseSystem::~DatabaseSystem() {
    for (const auto &[th, sess, queue, tid] : sess_list_) {
        if (queue)
            queue->Quit();
    }

    for (const auto &[th, sess, queue, tid] : sess_list_) {
        if (th && th->joinable())
            th->join();
    }
}

void DatabaseSystem::Init() {
    const auto &cfg = GetWorld()->GetServerConfig();

    const auto schemaName = cfg["database"]["mysql"]["schema"].as<std::string>();
    sess_list_ = std::vector<SessionNode>(cfg["database"]["pool"].as<uint64_t>());

    auto host = cfg["database"]["mysql"]["host"].as<std::string>();
    auto port = cfg["database"]["mysql"]["port"].as<uint16_t>();
    auto user = cfg["database"]["mysql"]["user"].as<std::string>();
    auto passwd = cfg["database"]["mysql"]["passwd"].as<std::string>();

    spdlog::info("{} - Connect To Database (MySQL){}:{}", __FUNCTION__, host, port);

    for (auto &node: sess_list_) {
        node.session = std::make_unique<mysqlx::Session>(host, port, user, passwd);
        node.queue = std::make_unique<ThreadSafeDeque<IDatabaseTask *>>();

        node.thread = std::make_unique<std::thread>([this, &node, schemaName] {
            node.threadID = std::this_thread::get_id();
            spdlog::info("\tThread[{}] - Begin Handle Database Task.", utils::ThreadIdToInt(node.threadID));
            while (node.queue->IsRunning()) {
                node.queue->Wait();
                if (!node.queue->IsRunning())
                    break;

                if (const auto op = node.queue->PopFront(); op.has_value() && op.value() != nullptr) {
                    const auto task = op.value();
                    try {
                        auto schema = node.session->getSchema(schemaName, true);
                        task->Execute(schema);
                    } catch (std::exception &e) {
                        spdlog::error("Database Error - {}", e.what());
                    }
                    delete task;
                }
            }

            node.queue->Clear();
            node.session->close();
        });
    }
}

void DatabaseSystem::SyncSelect(const std::string &tableName, const std::string &where, const std::function<void(mysqlx::Row)> &cb) const {
    if (sess_list_.empty()) {
        spdlog::error("{} - No Database Session Available.", __FUNCTION__);
        return;
    }

    const auto &[th, sess, queue, tid] = sess_list_.front();
    const auto &cfg = GetWorld()->GetServerConfig();

    auto schema = sess->getSchema(cfg["database"]["mysql"]["schema"].as<std::string>());
    if (!schema.existsInDatabase()) {
        spdlog::error("{} - schema not exists", __FUNCTION__);
        return;
    }

    auto table = schema.getTable(tableName);
    if (!table.existsInDatabase()) {
        spdlog::error("{} - table not exists", __FUNCTION__);
        return;
    }

    if (where.empty()) {
        for (auto result = table.select().execute(); const auto &row : result) {
            cb(row);
        }
    } else {
        for (auto result = table.select().where(where).execute(); const auto &row : result) {
            cb(row);
        }
    }
}

void DatabaseSystem::PushTransaction(const TransactionFunctor &func) {
    const auto &[th, sess, queue, tid] = sess_list_[next_node_idx_++];
    next_node_idx_ = next_node_idx_ % sess_list_.size();
    queue->PushBack(new TransactionTask(func));
}
