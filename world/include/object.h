#pragma once

#include "common.h"

#include <vector>
#include <concepts>

class BASE_API UObject {

    std::vector<UObject*> children_;

public:
    virtual ~UObject();

protected:
    template<class T, class... Args>
    requires std::derived_from<T, UObject>
    T *createObject(Args && ... args) {
        const auto res = new T(std::forward<Args>(args)...);
        children_.push_back(res);
        return res;
    }
};
