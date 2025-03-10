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

    [[nodiscard]] UGameWorld* getWorld() const;

    virtual void initGameWorld() = 0;

    virtual void setConnectionHandler(const std::shared_ptr<class UConnection> &conn) = 0;

    // 默认数据包编解码 如果使用了自定义数据包 那么需要重写这个函数 为Connection指定一个数据包编解码器
    virtual void setConnectionCodec(const std::shared_ptr<UConnection> &conn);

    virtual void setPackage();

    virtual void setPackagePool(class UMainScene *scene);
};
