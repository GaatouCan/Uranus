#pragma once

#include "../common.h"

#include <string>
#include <optional>

class UObject;

class BASE_API FClazzMethod final {

public:
    FClazzMethod() = delete;

    FClazzMethod(std::string name, uintptr_t ptr);
    ~FClazzMethod();

    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] uintptr_t GetPointer() const;

    template<class FuncType, class... Args>
    std::optional<std::invoke_result_t<FuncType, Args ...>> Invoke(UObject *obj, Args && ... args) const;

private:
    const std::string mName;
    uintptr_t mMethodPtr;
};

template<class FuncType, class ... Args>
std::optional<std::invoke_result_t<FuncType, Args ...>> FClazzMethod::Invoke(UObject *obj, Args &&...args) const {
    const auto func = reinterpret_cast<FuncType *>(mMethodPtr);
    if (func != nullptr) {
        using RetType = std::invoke_result_t<FuncType, Args...>;

        if constexpr (std::is_void_v<RetType>) {
            std::invoke(*func, obj, forward<Args>(args)...);
            return std::nullopt;
        } else {
            return std::make_optional<std::invoke_result_t<FuncType, Args ...>>(std::invoke(*func, obj, forward<Args>(args)...));
        }
    }
    return std::nullopt;
}