#include "../include/poolable.h"
#include "../include/recycler.h"


IPoolable::IPoolable(IRecycler *handle)
    : handle_(handle) {
}

IPoolable::~IPoolable() {
}

void IPoolable::recycle() {
    reset();
    handle_->recycle(this);
}
