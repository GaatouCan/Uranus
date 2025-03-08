#include "component_module.h"
#include "player.h"

#include "game_world.h"
#include "system/database/database_system.h"
#include "system/database/serializer.h"
#include "system/database/deserializer.h"

#include "../gameplay/component/appearance/appearance_ct.h"
#include "../gameplay/component/state/state_ct.h"


ComponentModule::ComponentModule(Player *plr)
    : owner_(plr){

    CreateComponent<StateCT>();
    CreateComponent<AppearanceCT>();
}

ComponentModule::~ComponentModule() {
    for (const auto &val : std::views::values(component_map_)) {
        delete val;
    }
}

Player * ComponentModule::GetOwner() const {
    return owner_;
}

void ComponentModule::OnDayChange() {
    for (const auto &val : component_map_ | std::views::values) {
        val->OnDayChange(false);
    }
}

void ComponentModule::Serialize() {
    auto ser = std::make_shared<USerializer>();

    for (const auto &val: std::views::values(component_map_)) {
        val->Serialize(ser);
    }

    if (const auto sys = GetOwner()->getWorld()->GetSystem<UDatabaseSystem>(); sys != nullptr) {
        sys->PushTransaction([ser, pid = owner_->getFullID()](mysqlx::Schema &schema) {
            ser->Serialize(schema);
            spdlog::info("ComponentModule::Serialize() - Player[{}] Stored.", pid);
            return true;
        });
    }
}

awaitable<void> ComponentModule::Deserialize() {
    try {
        AQueryArray query;
        std::string expr = fmt::format("pid = {}", GetOwner()->getFullID());

        for (const auto &val : component_map_ | std::views::values) {
            for (const auto &table : val->GetTableList()) {
                query.emplace_back(table, expr);
            }
        }

        if (const auto sys = GetOwner()->getWorld()->GetSystem<UDatabaseSystem>(); sys != nullptr) {
            if (const auto res = co_await sys->AsyncSelect(query, asio::use_awaitable); res != nullptr) {
                UDeserializer der(res);
                for (const auto &val : component_map_ | std::views::values) {
                    val->Deserialize(der);
                }
            }
        }
    } catch (const std::exception &e) {
        spdlog::error("{} - pid[{}] {}", __FUNCTION__, GetOwner()->getFullID(), e.what());
    }
}

void ComponentModule::OnLogin() {
    for (const auto &val: std::views::values(component_map_)) {
        val->OnLogin();
    }

    spdlog::info("{} - Player[{}] Loaded.", __FUNCTION__, GetOwner()->getFullID());
}

void ComponentModule::OnLogout() {
    for (const auto &val: std::views::values(component_map_)) {
        val->OnLogout();
    }
}

FPlayerID ComponentModule::GetPlayerID() const {
    if (owner_)
        return owner_->getPlayerID();
    return {};
}

void ComponentModule::SyncCache(CacheNode *node) {
    for (const auto &val: std::views::values(component_map_)) {
        val->SyncCache(node);
    }
}
