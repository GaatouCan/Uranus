#pragma once

#include "common.h"

#include <memory>

class BASE_API IServerLogic {

    class GameWorld* world_;

public:
    IServerLogic() = delete;

    explicit IServerLogic(GameWorld* world);
    virtual ~IServerLogic();

    DISABLE_COPY_MOVE(IServerLogic)

    [[nodiscard]] GameWorld* GetWorld() const;

    virtual void InitGameWorld() = 0;

    virtual void SetConnectionHandler(const std::shared_ptr<class Connection> &conn) = 0;
};
