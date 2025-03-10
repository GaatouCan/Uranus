#pragma once

#include "common.h"


class BASE_API IPoolable {

    friend class IRecycler;
    IRecycler* handle_;

protected:
    explicit IPoolable(IRecycler* handle);
    virtual ~IPoolable();

    virtual void initial() = 0;
    virtual void reset() = 0;

public:
    IPoolable() = delete;

    DISABLE_COPY_MOVE(IPoolable)

    virtual bool copyFrom(IPoolable *other) = 0;

    virtual void recycle();

    [[nodiscard]] virtual bool available() const = 0;
};
