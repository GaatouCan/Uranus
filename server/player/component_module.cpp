#include "component_module.h"
#include "player.h"

#include "game_world.h"
#include "system/database/database_system.h"

#include "../gameplay/component/appearance/appearance_ct.h"


ComponentModule::ComponentModule(Player *plr)
    : mOwner(plr){

    CreateComponent<AppearanceCT>();
}

ComponentModule::~ComponentModule() {
    for (const auto &ctx : std::views::values(mComponentMap)) {
        delete ctx;
    }
}

Player * ComponentModule::GetOwner() const {
    return mOwner;
}

void ComponentModule::OnDayChange() {
    for (const auto &ctx : mComponentMap | std::views::values) {
        ctx->GetComponent()->OnDayChange(false);
    }
}

void ComponentModule::Serialize() {
    IComponentContext::SerializerVector ret;

    for (const auto &ctx: std::views::values(mComponentMap)) {
        ctx->SerializeComponent(ret);
    }

    if (const auto sys = GetOwner()->GetWorld()->GetSystem<DatabaseSystem>(); sys != nullptr) {
        sys->PushTransaction([ret, pid = mOwner->GetFullID()](mysqlx::Schema &schema) {
            for (const auto &[serializer, bExpired] : ret) {
                if (serializer == nullptr)
                    continue;

                if (auto table = schema.getTable(serializer->GetTableName()); table.existsInDatabase()) {
                    if (bExpired) {
                        serializer->RemoveExpiredData(table, fmt::format("pid = {}", pid));
                    }
                    serializer->Serialize(table);
                }
                delete serializer;
            }
            return true;
        });
    }
}

awaitable<void> ComponentModule::Deserialize() {
    try {
        QueryVector query;
        std::string expr = fmt::format("pid = {}", GetOwner()->GetFullID());

        for (const auto &ctx : std::views::values(mComponentMap)) {
            for (const auto &name : ctx->GetTableList()) {
                query.emplace_back(name, expr);
            }
        }

        for (const auto &val : query) {
            spdlog::debug("{} - {} {}", __FUNCTION__, val.first, val.second);
        }

        if (const auto sys = GetOwner()->GetWorld()->GetSystem<DatabaseSystem>(); sys != nullptr) {
            if (const auto res = co_await sys->AsyncSelect(query, asio::use_awaitable); res != nullptr) {
                for (const auto &ctx : std::views::values(mComponentMap)) {
                    ctx->DeserializeComponent(*res);
                }
            }
        }
    } catch (const std::exception &e) {
        spdlog::error("{} - pid[{}] {}", __FUNCTION__, GetOwner()->GetFullID(), e.what());
    }
}

void ComponentModule::OnLogin() {
    for (const auto &ctx: std::views::values(mComponentMap)) {
        ctx->GetComponent()->OnLogin();
    }
}

void ComponentModule::OnLogout() {
    for (const auto &ctx: std::views::values(mComponentMap)) {
        ctx->GetComponent()->OnLogout();
    }
}

PlayerID ComponentModule::GetPlayerID() const {
    if (mOwner)
        return mOwner->GetPlayerID();
    return {};
}

void ComponentModule::SyncCache(CacheNode *node) {
    for (const auto &ctx: std::views::values(mComponentMap)) {
        ctx->GetComponent()->SyncCache(node);
    }
}
