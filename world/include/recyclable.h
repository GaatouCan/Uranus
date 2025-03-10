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

    virtual void initial() = 0;
    virtual void reset() = 0;

public:
    IRecyclable() = delete;

    DISABLE_COPY_MOVE(IRecyclable)

    virtual bool copyFrom(IRecyclable *other);

    [[nodiscard]] virtual bool available() const = 0;

    virtual void recycle();
};
