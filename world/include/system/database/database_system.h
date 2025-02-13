#pragma once

#include "../../sub_system.h"
#include "utils.h"

#include <memory>
#include <thread>
#include <mysqlx/xdevapi.h>


class BASE_API DatabaseSystem final : public ISubSystem {

    struct SessionNode {
        std::unique_ptr<std::thread> thread;
        std::unique_ptr<mysqlx::Session> session;
        ThreadID threadID;
    };

    std::vector<SessionNode> mSessionList;
    std::atomic_size_t mNextNodeIndex = 0;

public:
    explicit DatabaseSystem(GameWorld *world);
    ~DatabaseSystem() override;

    GET_SYSTEM_NAME(DatabaseSystem)

    void Init() override;
};
