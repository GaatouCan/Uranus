#include "component_module.h"
#include "player.h"

#include "game_world.h"
#include "system/database/database_system.h"
#include "system/database/serializer.h"
#include "system/database/deserializer.h"

#include "../gameplay/component/appearance/appearance_ct.h"
#include "../gameplay/component/state/state_ct.h"


UComponentModule::UComponentModule(UPlayer *plr)
    : owner_(plr){

    createComponent<UStateCT>();
    createComponent<UAppearanceCT>();
}

UComponentModule::~UComponentModule() {
    for (const auto &val : std::views::values(componentMap_)) {
        delete val;
    }
}

UPlayer * UComponentModule::getOwner() const {
    return owner_;
}

void UComponentModule::onDayChange() {
    for (const auto &val : componentMap_ | std::views::values) {
        val->onDayChange(false);
    }
}

void UComponentModule::serialize() {
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

awaitable<void> UComponentModule::deserialize() {
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

void UComponentModule::onLogin() {
    for (const auto &val: std::views::values(componentMap_)) {
        val->onLogin();
    }

    spdlog::info("{} - Player[{}] Loaded.", __FUNCTION__, getOwner()->getFullID());
}

void UComponentModule::onLogout() {
    for (const auto &val: std::views::values(componentMap_)) {
        val->onLogout();
    }
}

FPlayerID UComponentModule::getPlayerID() const {
    if (owner_)
        return owner_->getPlayerID();
    return {};
}

void UComponentModule::syncCache(FCacheNode *node) {
    for (const auto &val: std::views::values(componentMap_)) {
        val->syncCache(node);
    }
}
