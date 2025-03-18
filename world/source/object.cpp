#include "../include/object.h"

UObject::UObject() {
}

UObject::UObject(UObject *parent)
    : parent_(parent) {
}

UObject::~UObject() {
    for (const auto &child : children_) {
        delete child;
    }
}

UObject * UObject::getParent() const {
    return parent_;
}
