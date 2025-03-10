#pragma once

#include "common.h"

#ifdef __linux__
#include <cstdint>
#endif

class BASE_API IRecyclable {

    friend class IRecycler;
    IRecycler* handle_;

protected:
    explicit IRecyclable(IRecycler* handle);
    virtual ~IRecyclable();

    // IRecycler::acquire() 时被调用
    virtual void initial() = 0;

    // IRecycler::recycle() 时被调用
    virtual void reset() = 0;

public:
    IRecyclable() = delete;

    DISABLE_COPY_MOVE(IRecyclable)

    // 自定义深拷贝数据部分
    virtual bool copyFrom(IRecyclable *other);

    // IRecycler::acquire() 之后未赋值之前为true
    [[nodiscard]] virtual bool unused() const = 0;

    // IRecyclable::recycle() 之后为false
    [[nodiscard]] virtual bool available() const = 0;

    virtual void recycle();
};
