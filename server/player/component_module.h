#pragma once

#include "player_id.h"

#include "player_component.h"
#include "system/database/serializer.h"
#include "system/database/deserializer.h"

#include <spdlog/spdlog.h>
#include <typeindex>
#include <concepts>
#include <ranges>


class ComponentModule;

class IComponentContext {

protected:

    IPlayerComponent* mComponent = nullptr;
    ComponentModule* mModule = nullptr;

public:

    using SerializerVector = std::vector<std::pair<ISerializer *, bool>>;
    using DeserializerMap = std::unordered_map<std::string, mysqlx::RowResult>;

    IComponentContext() = delete;

    explicit IComponentContext(ComponentModule *module) : mModule(module) {}
    virtual ~IComponentContext() {
        delete mComponent;
    }

    [[nodiscard]] ComponentModule* GetModule() const { return mModule; }
    [[nodiscard]] IPlayerComponent* GetComponent() const { return mComponent; }

    [[nodiscard]] virtual std::vector<std::string> GetTableList() const = 0;

    virtual void SerializeComponent(SerializerVector &vec) const = 0;
    virtual void DeserializeComponent(DeserializerMap &map) const = 0;
};

template<class Component>
requires std::derived_from<Component, IPlayerComponent>
class TComponentContext final : public IComponentContext {

public:
    using SerializeFunctor = ISerializer*(Component::*)(bool &) const;
    using DeserializeFunctor = void(Component::*)(Deserializer &);

    explicit TComponentContext(ComponentModule *module) : IComponentContext(module) {
        mComponent = new Component(this);
    }

    void RegisterTable(const std::string &table, SerializeFunctor ser, DeserializeFunctor deser) {
        mTableMap.insert_or_assign(table, std::make_pair(ser, deser));
    }

    [[nodiscard]] std::vector<std::string> GetTableList() const override {
        std::vector<std::string> result;
        for (const auto &name : mTableMap | std::views::keys) {
            result.emplace_back(name);
        }
        return result;
    }

    void SerializeComponent(SerializerVector &vec) const override {
        for (auto &[s, ds] : mTableMap | std::views::values) {
            if (s) {
                bool bExpired = false;
                ISerializer *is = std::invoke(s, dynamic_cast<Component *>(mComponent), bExpired);
                vec.emplace_back(is, bExpired);
            }
        }
    }

    void DeserializeComponent(DeserializerMap &map) const override {
        for (auto &[name, pair] : mTableMap) {
            if (const auto it = map.find(name); it != map.end()) {
                Deserializer ds(std::move(it->second));
                std::invoke(pair.second, dynamic_cast<Component *>(mComponent), ds);
            }
        }
    }

private:
    std::unordered_map<std::string, std::pair<SerializeFunctor, DeserializeFunctor>> mTableMap;
};


class ComponentModule final {

    Player *mOwner;
    std::unordered_map<std::type_index, IComponentContext *> mComponentMap;

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
        mComponentMap.insert_or_assign(typeid(T), ctx);

        spdlog::debug("{} - Player[{}] load {}", __FUNCTION__, GetPlayerID().ToInt64(), ctx->GetComponent()->GetComponentName());

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
