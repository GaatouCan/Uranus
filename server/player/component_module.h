#pragma once

#include "player_id.h"

#include "player_component.h"
#include "system/database/table_array.h"
#include "system/database/table_result.h"

#include <spdlog/spdlog.h>
#include <typeindex>
#include <concepts>
#include <ranges>
#include <asio/awaitable.hpp>


class ComponentModule;

class IComponentContext {

protected:

    IPlayerComponent* mComponent = nullptr;
    ComponentModule* mModule = nullptr;

public:

    using SerializerVector = std::vector<std::pair<ITableArray *, bool>>;
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
    using SerializeFunctor = ITableArray*(Component::*)(bool &) const;
    using DeserializeFunctor = void(Component::*)(TableResult &);

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
                if (ITableArray *is = std::invoke(s, dynamic_cast<Component *>(mComponent), bExpired); is != nullptr) {
                    vec.emplace_back(is, bExpired);
                }
            }
        }
    }

    void DeserializeComponent(DeserializerMap &map) const override {
        for (auto &[name, pair] : mTableMap) {
            if (const auto it = map.find(name); it != map.end()) {
                TableResult ds(std::move(it->second));
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

    void Serialize();
    asio::awaitable<void> Deserialize();

    void OnLogin();
    void OnLogout();

    [[nodiscard]] PlayerID GetPlayerID() const;

    void SyncCache(CacheNode *node);
};


/**
 * 将数据写入数据库（序列化）
 * @param tb 数据库表名
 * @param pa 数据块
 */
#define WRITE_TABLE(ser, tb, pa) \
{ \
    auto *array = (ser)->CreateTableVector<orm::DBTable_##tb>(utils::PascalToUnderline(#tb)); \
    array->PushBack(pa); \
}

/**
 * 将数据写入数据库（序列化）（key-value结构）
 * @param tb 数据库表名
 * @param pa 包含数据块的map
 */
#define WRITE_TABLE_MAP(ser, tb, pa) \
{ \
    auto *array = (ser)->CreateTableVector<orm::DBTable_##tb>(utils::PascalToUnderline(#tb)); \
    for (const auto &val : (pa) | std::views::values) { \
        array->PushBack(val); \
    } \
}

/**
 * 将数据写入数据库（序列化）（array结构）
 * @param tb 数据库表名
 * @param pa 包含数据块的vector
 */
#define WRITE_TABLE_VECTOR(ser, tb, pa) \
{ \
    auto *array = (ser)->CreateTableVector<orm::DBTable_##tb>(utils::PascalToUnderline(#tb)); \
    for (const auto &val : (pa)) { \
        array->PushBack(val); \
    } \
}

/**
 * 将数据库的数据读取到数据块
 * @param ds Deserializer对象
 * @param pa 数据块
 */
#define READ_TABLE(ds, tb, pa) \
if (auto res = (ds)->FetchResult(utils::PascalToUnderline(#tb)); res.has_value()) { \
    if (res->HasMore()) { \
        res->Deserialize(&pa); \
    } \
}

/**
 * 将数据库的数据读取到包含数据块的map
 * @param ds Deserializer对象
 * @param pa 包含数据块的map
 */
#define READ_TABLE_MAP(ds, tb, pa) \
if (auto res = (ds)->FetchResult(utils::PascalToUnderline(#tb)); res.has_value()) { \
    while (res->HasMore()) { \
        decltype(pa)::mapped_type val; \
        res->Deserialize(&val); \
        (pa)[val.index] = std::move(val); \
    } \
}

/**
 * 将数据库的数据读取到包含数据块的vector
 * @param ds Deserializer对象
 * @param pa 包含数据块的vector
 */
#define READ_TABLE_VECTOR(ds, pa) \
if (auto res = (ds)->FetchResult(utils::PascalToUnderline(#tb)); res.has_value()) { \
    (pa).resize((ds).TotalRowsCount());\
    while (res->HasMore()) { \
        decltype(pa)::mapped_type val; \
        res->Deserialize(&val); \
        (pa)[val.index] = std::move(val); \
    } \
}
