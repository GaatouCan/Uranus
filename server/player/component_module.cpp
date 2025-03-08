#include "component_module.h"
#include "player.h"

#include "game_world.h"
#include "system/database/database_system.h"
#include "system/database/serializer.h"
#include "system/database/deserializer.h"

#include "../gameplay/component/appearance/appearance_ct.h"
#include "../gameplay/component/state/state_ct.h"


ComponentModule::ComponentModule(UPlayer *plr)
    : owner_(plr){

    createComponent<StateCT>();
    createComponent<AppearanceCT>();
}

ComponentModule::~ComponentModule() {
    for (const auto &val : std::views::values(componentMap_)) {
        delete val;
    }
}

UPlayer * ComponentModule::getOwner() const {
    return owner_;
}

void ComponentModule::onDayChange() {
    for (const auto &val : componentMap_ | std::views::values) {
        val->onDayChange(false);
    }
}

void ComponentModule::serialize() {
    auto ser = std::make_shared<USerializer>();

    for (const auto &val: std::views::values(componentMap_)) {
        val->serialize(ser);
    }

    if (const auto sys = getOwner()->getWorld()->getSystem<UDatabaseSystem>(); sys != nullptr) {
        sys->pushTransaction([ser, pid = owner_->getFullID()](mysqlx::Schema &schema) {
            ser->serialize(schema);
            spdlog::info("ComponentModule::Serialize() - Player[{}] Stored.", pid);
            return true;
        });
    }
}

awaitable<void> ComponentModule::deserialize() {
    try {
        AQueryArray query;
        std::string expr = fmt::format("pid = {}", getOwner()->getFullID());

        for (const auto &val : componentMap_ | std::views::values) {
            for (const auto &table : val->getTableList()) {
                query.emplace_back(table, expr);
            }
        }

        if (const auto sys = getOwner()->getWorld()->getSystem<UDatabaseSystem>(); sys != nullptr) {
            if (const auto res = co_await sys->asyncSelect(query, asio::use_awaitable); res != nullptr) {
                UDeserializer der(res);
                for (const auto &val : componentMap_ | std::views::values) {
                    val->deserialize(der);
                }
            }
        }
    } catch (const std::exception &e) {
        spdlog::error("{} - pid[{}] {}", __FUNCTION__, getOwner()->getFullID(), e.what());
    }
}

void ComponentModule::onLogin() {
    for (const auto &val: std::views::values(componentMap_)) {
        val->onLogin();
    }

    spdlog::info("{} - Player[{}] Loaded.", __FUNCTION__, getOwner()->getFullID());
}

void ComponentModule::onLogout() {
    for (const auto &val: std::views::values(componentMap_)) {
        val->onLogout();
    }
}

FPlayerID ComponentModule::getPlayerID() const {
    if (owner_)
        return owner_->getPlayerID();
    return {};
}

void ComponentModule::syncCache(CacheNode *node) {
    for (const auto &val: std::views::values(componentMap_)) {
        val->syncCache(node);
    }
}
