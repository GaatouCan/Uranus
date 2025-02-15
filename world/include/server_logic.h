#pragma once

#include "common.h"

#include <memory>

class BASE_API IServerLogic {

    class GameWorld* mWorld;

public:
    IServerLogic() = delete;

    explicit IServerLogic(GameWorld* world);
    virtual ~IServerLogic();

    DISABLE_COPY_MOVE(IServerLogic)

    [[nodiscard]] GameWorld* GetWorld() const;

    virtual void InitGameWorld() = 0;

    virtual void SetConnectionHandler(const std::shared_ptr<class Connection> &conn) = 0;

    // 默认数据包编解码 如果使用了自定义数据包 那么需要重写这个函数 为Connection指定一个数据包编解码器
    virtual void SetConnectionCodec(const std::shared_ptr<Connection> &conn);
};
