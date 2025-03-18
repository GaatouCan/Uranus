#pragma once

#include "common.h"

#include <vector>
#include <concepts>

class BASE_API UObject {

    UObject *parent_ = nullptr;
    std::vector<UObject *> children_;

public:
    UObject();
    explicit UObject(UObject *parent);

    virtual ~UObject();

protected:
    template<class T, class... Args>
    requires std::derived_from<T, UObject>
    T *createObject(Args && ... args) {
        const auto res = new T(std::forward<Args>(args)...);
        children_.push_back(res);
        return res;
    }

    template<class T, class... Args>
    requires std::derived_from<T, UObject>
    T *createSubObject(Args && ... args) {
        const auto res = new T(this, std::forward<Args>(args)...);
        children_.push_back(res);
        return res;
    }

    [[nodiscard]] UObject *getParent() const;

    template<class T>
    requires std::derived_from<T, UObject>
    T *getParentT() const {
        if (parent_ != nullptr)
            return dynamic_cast<T *>(parent_);
        return nullptr;
    }
};
