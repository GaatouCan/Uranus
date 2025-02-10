#pragma once

#include "common.h"

#include <memory>

class BASE_API IServerLogic {

    class UGameWorld* world_;

public:
    IServerLogic() = delete;

    explicit IServerLogic(UGameWorld* world);
    virtual ~IServerLogic();

    DISABLE_COPY_MOVE(IServerLogic)

    [[nodiscard]] UGameWorld* GetWorld() const;

    virtual void InitGameWorld() = 0;

    virtual void SetConnectionHandler(const std::shared_ptr<class UConnection> &conn) = 0;
};
