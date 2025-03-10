#include "../include/recyclable.h"
#include "../include/recycler.h"


IRecyclable::IRecyclable(IRecycler *handle)
    : handle_(handle) {
}

IRecyclable::~IRecyclable() {
}

bool IRecyclable::copyFrom(IRecyclable *other) {
    if (other != nullptr && other != this) {
        handle_ = other->handle_;
    }
    return false;
}

void IRecyclable::recycle() {
    handle_->recycle(this);
}
