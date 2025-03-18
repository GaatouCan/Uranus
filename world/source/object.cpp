#include "../include/object.h"

UObject::~UObject() {
    for (const auto &child : children_) {
        delete child;
    }
}
