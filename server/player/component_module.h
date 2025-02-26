#pragma once

#include "player_id.h"

#include "player_component.h"
#include "system/database/table_array.h"

#include <spdlog/spdlog.h>
#include <typeindex>
#include <concepts>
#include <ranges>
#include <asio/awaitable.hpp>


class ComponentModule final {

    Player *mOwner;
    std::unordered_map<std::type_index, IPlayerComponent *> mComponentMap;

public:
    ComponentModule() = delete;

    explicit ComponentModule(Player *plr);
    ~ComponentModule();

    [[nodiscard]] Player *GetOwner() const;

    void OnDayChange();

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *CreateComponent() {
        const auto res = new T(this);
        mComponentMap[typeid(T)] = res;

        return res;
    }

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *GetComponent() {
        if (const auto it = mComponentMap.find(typeid(T)); it != mComponentMap.end()) {
            return dynamic_cast<T *>(it->second);
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
if (auto *res = (ds)->FetchResult(utils::PascalToUnderline(#tb)); res != nullptr) { \
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
if (auto *res = (ds)->FetchResult(utils::PascalToUnderline(#tb)); res != nullptr) { \
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
#define READ_TABLE_VECTOR(ds, tb, pa) \
if (auto *res = (ds)->FetchResult(utils::PascalToUnderline(#tb)); res != nullptr) { \
    (pa).resize((ds).TotalRowsCount());\
    while (res->HasMore()) { \
        decltype(pa)::mapped_type val; \
        res->Deserialize(&val); \
        (pa)[val.index] = std::move(val); \
    } \
}
