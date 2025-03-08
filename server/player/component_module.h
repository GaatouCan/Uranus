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

    UPlayer *owner_;
    std::unordered_map<std::type_index, IPlayerComponent *> componentMap_;

public:
    ComponentModule() = delete;

    explicit ComponentModule(UPlayer *plr);
    ~ComponentModule();

    [[nodiscard]] UPlayer *getOwner() const;

    void onDayChange();

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *createComponent() {
        const auto res = new T(this);
        componentMap_[typeid(T)] = res;

        return res;
    }

    template<typename T>
    requires std::derived_from<T, IPlayerComponent>
    T *getComponent() {
        if (const auto it = componentMap_.find(typeid(T)); it != componentMap_.end()) {
            return dynamic_cast<T *>(it->second);
        }
        return nullptr;
    }

    void serialize();
    asio::awaitable<void> deserialize();

    void onLogin();
    void onLogout();

    [[nodiscard]] FPlayerID getPlayerID() const;

    void syncCache(CacheNode *node);
};


/**
 * 将数据写入数据库（序列化）
 * @param tb 数据库表名
 * @param pa 数据块
 */
#define WRITE_TABLE(ser, tb, pa) \
{ \
    auto *array = (ser)->createTableVector<orm::DBTable_##tb>(utils::PascalToUnderline(#tb)); \
    array->pushBack(pa); \
}

/**
 * 将数据写入数据库（序列化）（key-value结构）
 * @param tb 数据库表名
 * @param pa 包含数据块的map
 */
#define WRITE_TABLE_MAP(ser, tb, pa) \
{ \
    auto *array = (ser)->createTableVector<orm::DBTable_##tb>(utils::PascalToUnderline(#tb)); \
    for (const auto &val : (pa) | std::views::values) { \
        array->pushBack(val); \
    } \
}

/**
 * 将数据写入数据库（序列化）（array结构）
 * @param tb 数据库表名
 * @param pa 包含数据块的vector
 */
#define WRITE_TABLE_VECTOR(ser, tb, pa) \
{ \
    auto *array = (ser)->createTableVector<orm::DBTable_##tb>(utils::PascalToUnderline(#tb)); \
    for (const auto &val : (pa)) { \
        array->pushBack(val); \
    } \
}

/**
 * 将数据库的数据读取到数据块
 * @param ds Deserializer对象
 * @param pa 数据块
 */
#define READ_TABLE(ds, tb, pa) \
if (auto *res = (ds)->fetchResult(utils::PascalToUnderline(#tb)); res != nullptr) { \
    if (res->hasMore()) { \
        res->deserialize(&pa); \
    } \
}

/**
 * 将数据库的数据读取到包含数据块的map
 * @param ds Deserializer对象
 * @param pa 包含数据块的map
 */
#define READ_TABLE_MAP(ds, tb, pa) \
if (auto *res = (ds)->fetchResult(utils::PascalToUnderline(#tb)); res != nullptr) { \
    while (res->hasMore()) { \
        decltype(pa)::mapped_type val; \
        res->deserialize(&val); \
        (pa)[val.index] = std::move(val); \
    } \
}

/**
 * 将数据库的数据读取到包含数据块的vector
 * @param ds Deserializer对象
 * @param pa 包含数据块的vector
 */
#define READ_TABLE_VECTOR(ds, tb, pa) \
if (auto *res = (ds)->fetchResult(utils::PascalToUnderline(#tb)); res != nullptr) { \
    (pa).resize((ds).totalRowsCount());\
    while (res->hasMore()) { \
        decltype(pa)::value_type val; \
        res->deserialize(&val); \
        (pa)[val.index] = std::move(val); \
    } \
}
