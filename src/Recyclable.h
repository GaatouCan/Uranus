#pragma once

#include "common.h"

#include <memory>
#ifdef __linux__
#include <cstdint>
#endif

/**
 * The Base Class Of Recyclable Object,
 * The Element Managed By Recycler
 */
class BASE_API IRecyclable {

    friend class IRecycler;

protected:
    /** Called When It Created By Recycler */
    virtual void OnCreate() {}

    /** Called When Acquiring From Recycler */
    virtual void Initial() = 0;

    /** Called When It Recycled To Recycler */
    virtual void Reset() = 0;

public:
    IRecyclable() = default;
    virtual ~IRecyclable() = default;

    DISABLE_COPY_MOVE(IRecyclable)

    /** Depth Copy Object Data */
    virtual bool CopyFrom(IRecyclable *other);

    /** Depth Copy Object Data */
    virtual bool CopyFrom(const std::shared_ptr<IRecyclable> &other);

    /** After Acquiring And Before Assigning Return True */
    [[nodiscard]] virtual bool IsUnused() const = 0;

    /** After Recycling It Was False */
    [[nodiscard]] virtual bool IsAvailable() const = 0;
};
