#include "../../../include/system/database/database_system.h"
#include "game_world.h"
#include "../../../include/config_manager.h"

#include <spdlog/spdlog.h>

DatabaseSystem::DatabaseSystem(GameWorld *world)
    : ISubSystem(world) {
}

DatabaseSystem::~DatabaseSystem() {

    for (auto &[th, sess, tid] : mSessionList) {
        if (th && th->joinable())
            th->join();
    }
}

void DatabaseSystem::Init() {
    const auto &cfg = GetWorld()->GetServerConfig();

    const auto schemaName = cfg["database"]["mysql"]["schema"].as<std::string>();
    mSessionList = std::vector<SessionNode>(cfg["database"]["pool"].as<uint64_t>());

    for (auto &node: mSessionList) {
        node.sess = std::make_unique<mysqlx::Session>(
            cfg["database"]["mysql"]["host"].as<std::string>(),
            cfg["database"]["mysql"]["port"].as<uint16_t>(),
            cfg["database"]["mysql"]["user"].as<std::string>(),
            cfg["database"]["mysql"]["passwd"].as<std::string>()
        );
        // node.queue = std::make_unique<TSDeque<IDatabaseWrapper *>>();

        node.th = std::make_unique<std::thread>([this, &node, &schemaName] {
            node.tid = std::this_thread::get_id();
            spdlog::info("\tThread ID {} - Begin handle database task", utils::ThreadIdToInt(node.tid));
            // while (node.queue->IsRunning()) {
            //     node.queue->Wait();
            //     if (!node.queue->IsRunning())
            //         break;
            //
            //     if (const auto wrapper = node.queue->PopFront(); wrapper != nullptr) {
            //         if (auto schema = node.sess->getSchema(schemaName); schema.existsInDatabase()) {
            //             wrapper->Execute(schema);
            //         }
            //         delete wrapper;
            //     }
            // }
            //
            // node.queue->Clear();
            node.sess->close();
        });
    }
}
