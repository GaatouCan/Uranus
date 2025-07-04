#include "Recyclable.h"

bool IRecyclable::CopyFrom(IRecyclable *other) {
    if (other != nullptr && other != this) {
        return true;
    }
    return false;
}

bool IRecyclable::CopyFrom(const std::shared_ptr<IRecyclable> &other) {
    if (other != nullptr && other.get() != this) {
        return true;
    }
    return false;
}
