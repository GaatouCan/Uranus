#include "../include/poolable.h"
#include "../include/recycler.h"


IPoolable::IPoolable(uranus::internal::IRecycler *handle)
    : handle_(handle) {
}

IPoolable::~IPoolable() {
}

void IPoolable::recycle() {
    reset();
    handle_->recycle(this);
}
