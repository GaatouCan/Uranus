#pragma once

#include <typeindex>
#include <concepts>
// #include <mysqlx/xdevapi.h>
#include <spdlog/spdlog.h>
#include <PlayerID.h>

#include "PlayerComponent.h"
// #include <system/database/Serializer.h>
// #include <system/database/Deserializer.h>

#include <ranges>


class UComponentModule;

class IComponentContext {

protected:
    IPlayerComponent* mComponent = nullptr;
    UComponentModule* mModule = nullptr;

public:
    // using ASerializerVector = std::vector<std::pair<ISerializer *, bool>>;
    // using ADeserializerMap = std::unordered_map<std::string, mysqlx::RowResult>;

    IComponentContext() = delete;

    explicit IComponentContext(UComponentModule *module) : mModule(module) {}
    virtual ~IComponentContext() {
        delete mComponent;
    }

    [[nodiscard]] UComponentModule* GetModule() const { return mModule; }
    [[nodiscard]] IPlayerComponent* GetComponent() const { return mComponent; }

    // [[nodiscard]] virtual std::vector<std::string> GetTableList() const = 0;

    // virtual void SerializeComponent(ASerializerVector &vec) const = 0;
    // virtual void DeserializeComponent(ADeserializerMap &map) const = 0;
};

template<class Component>
requires std::derived_from<Component, IPlayerComponent>
class TComponentContext final : public IComponentContext {

public:
    // using ASerializeFunctor = ISerializer*(Component::*)(bool &) const;
    // using ADeserializeFunctor = void(Component::*)(UDeserializer &);

    explicit TComponentContext(UComponentModule *module) : IComponentContext(module) {
        mComponent = new Component(this);
    }

    // void RegisterTable(const std::string &table, ASerializeFunctor ser, ADeserializeFunctor deser) {
    //     mMap.insert_or_assign(table, std::make_pair(ser, deser));
    // }
    //
    // [[nodiscard]] std::vector<std::string> GetTableList() const override {
    //     std::vector<std::string> result;
    //     for (const auto &name : mMap | std::views::keys) {
    //         result.emplace_back(name);
    //     }
    //     return result;
    // }
    //
    // void SerializeComponent(ASerializerVector &vec) const override {
    //     for (auto &[s, ds] : mMap | std::views::values) {
    //         if (s) {
    //             bool bExpired = false;
    //             ISerializer *is = std::invoke(s, dynamic_cast<Component *>(mComponent), bExpired);
    //             vec.emplace_back(is, bExpired);
    //         }
    //     }
    // }
    //
    // void DeserializeComponent(ADeserializerMap &map) const override {
    //     for (auto &[name, pair] : mMap) {
    //         if (const auto it = map.find(name); it != map.end()) {
    //             UDeserializer ds(std::move(it->second));
    //             std::invoke(pair.second, dynamic_cast<Component *>(mComponent), ds);
    //         }
    //     }
    // }

private:
    // std::map<std::string, std::pair<ASerializeFunctor, ADeserializeFunctor>> mMap;
};


class UComponentModule final {

    UPlayer *mOwner;

    std::map<std::type_index, IComponentContext *> mComponentMap;

public:
    UComponentModule() = delete;

    explicit UComponentModule(UPlayer *plr);
    ~UComponentModule();

    [[nodiscard]] UPlayer *GetOwner() const;

    void OnDayChange();

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *CreateComponent() {
        const auto ctx = new TComponentContext<T>(this);
        mComponentMap.insert_or_assign(typeid(T), ctx);

        spdlog::debug("{} - Player[{}] load {}", __FUNCTION__, GetPlayerID().ToUInt64(), ctx->GetComponent()->GetComponentName());

        return dynamic_cast<T *>(ctx->GetComponent());
    }

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *GetComponent() {
        if (const auto it = mComponentMap.find(typeid(T)); it != mComponentMap.end()) {
            return dynamic_cast<T *>(it->second->GetComponent());
        }
        return nullptr;
    }

    // void Serialize();
    // awaitable<void> Deserialize();

    void OnLogin();
    void OnLogout();

    [[nodiscard]] PlayerID GetPlayerID() const;

    void SyncCache(FCacheNode *node);
};

#define CREATE_SERIALIZER(s, table) const auto s = new USerializer<orm::UDBTable_##table>(utils::PascalToUnderline(#table));

/**
 * 注册组件序列化和反序列化调用
 * @param comp 组件类型
 * @param tb 数据库表名
 */
#define SERIALIZE_COMPONENT(comp, tb) \
dynamic_cast<TComponentContext<comp> *>(GetComponentContext())->RegisterTable(utils::PascalToUnderline(#tb), &comp::Serialize_##tb, &comp::Deserialize_##tb);

#define READ_PARAM(tb, pa) \
CREATE_SERIALIZER(s, tb) \
s->PushBack(pa); \
return s;

#define READ_PARAM_MAP(tb, pa) \
CREATE_SERIALIZER(s, tb) \
for (const auto &val : (pa) | std::views::values) { \
    s->PushBack(val); \
} \
return s;

#define WRITE_PARAM(ds, pa) \
if ((ds).HasMore()) { \
    (ds).Deserialize(&(pa)); \
}

#define WRITE_PARAM_MAP(ds, pa) \
while ((ds).HasMore()) { \
    decltype(pa)::mapped_type val; \
    (ds).Deserialize(&val); \
    (pa)[val.index] = val; \
}
