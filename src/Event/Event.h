#pragma once

#include "Common.h"

#include <concepts>


class BASE_API IEventInterface {
public:
    IEventInterface() = default;
    virtual ~IEventInterface() = default;

    [[nodiscard]] virtual int GetEventType() const = 0;
};

template <typename T>
concept CEventType = std::derived_from<T, IEventInterface>;


#include <nlohmann/json.hpp>

class BASE_API UDefaultEvent final : public IEventInterface {
public:
    [[nodiscard]] int GetEventType() const override;

    void SetEventParam(const nlohmann::json &data);
    [[nodiscard]] const nlohmann::json &GetEventParam() const;

private:
    nlohmann::json mData;
};
