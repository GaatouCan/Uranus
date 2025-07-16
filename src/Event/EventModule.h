#pragma once

#include "Module.h"
#include "EventParam.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <memory>
#include <shared_mutex>


using absl::flat_hash_map;
using absl::flat_hash_set;


class BASE_API UEventModule final : public IModuleBase {

    DECLARE_MODULE(UEventModule)

protected:
    explicit UEventModule(UServer *server);

public:
    ~UEventModule() override = default;

    constexpr const char *GetModuleName() const override {
        return "Event Module";
    }

    template<CEventType Type>
    std::shared_ptr<Type> CreateEventParam() const;

    void Dispatch(const std::shared_ptr<IEventParam> &event) const;

    template<CEventType Type, class... Args>
    void DispatchT(Args && ... args);

    void ListenEvent(int event, int32_t sid, int64_t pid = -1);
    void RemoveListener(int event, int32_t sid, int64_t pid = -1);

private:
    flat_hash_map<int, flat_hash_set<int32_t>> mServiceSet;
    flat_hash_map<int, flat_hash_set<int64_t>> mPlayerSet;
    mutable std::shared_mutex mMutex;
};

template<CEventType Type>
inline std::shared_ptr<Type> UEventModule::CreateEventParam() const {
    if (mState != EModuleState::RUNNING)
        return nullptr;

    auto result = std::make_shared<Type>();
    return result;
}

template<CEventType Type, class ... Args>
inline void UEventModule::DispatchT(Args &&...args) {
    if (mState != EModuleState::RUNNING)
        return;

    auto res = std::make_shared<Type>(std::forward<Args>(args)...);
    this->Dispatch(res);
}
