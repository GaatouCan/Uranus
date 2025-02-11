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


class ComponentModule;

class IComponentContext {

protected:

    IPlayerComponent* component_ = nullptr;
    ComponentModule* module_ = nullptr;

public:
    // using ASerializerVector = std::vector<std::pair<ISerializer *, bool>>;
    // using ADeserializerMap = std::unordered_map<std::string, mysqlx::RowResult>;

    IComponentContext() = delete;

    explicit IComponentContext(ComponentModule *module) : module_(module) {}
    virtual ~IComponentContext() {
        delete component_;
    }

    [[nodiscard]] ComponentModule* GetModule() const { return module_; }
    [[nodiscard]] IPlayerComponent* GetComponent() const { return component_; }

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

    explicit TComponentContext(ComponentModule *module) : IComponentContext(module) {
        component_ = new Component(this);
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


class ComponentModule final {

    Player *owner_;

    std::map<std::type_index, IComponentContext *> component_map_;

public:
    ComponentModule() = delete;

    explicit ComponentModule(Player *plr);
    ~ComponentModule();

    [[nodiscard]] Player *GetOwner() const;

    void OnDayChange();

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *CreateComponent() {
        const auto ctx = new TComponentContext<T>(this);
        component_map_.insert_or_assign(typeid(T), ctx);

        spdlog::debug("{} - Player[{}] load {}", __FUNCTION__, GetPlayerID().ToInt64(), ctx->GetComponent()->GetComponentName());

        return dynamic_cast<T *>(ctx->GetComponent());
    }

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *GetComponent() {
        if (const auto it = component_map_.find(typeid(T)); it != component_map_.end()) {
            return dynamic_cast<T *>(it->second->GetComponent());
        }
        return nullptr;
    }

    // void Serialize();
    // awaitable<void> Deserialize();

    void OnLogin();
    void OnLogout();

    [[nodiscard]] PlayerID GetPlayerID() const;

    void SyncCache(CacheNode *node);
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
