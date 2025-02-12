#include "component_module.h"
#include "player.h"

// #include "../world/component/appearance/AppearanceCT.h"

#include <game_world.h>
// #include <system/database/DatabaseSystem.h>


ComponentModule::ComponentModule(Player *plr)
    : mOwner(plr){

    // CreateComponent<UAppearanceCT>();
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

// void UComponentModule::Serialize() {
//     IComponentContext::ASerializerVector ret;
//
//     for (const auto &ctx: std::views::values(mComponentMap)) {
//         ctx->SerializeComponent(ret);
//     }
//
//     if (const auto sys = UGameWorld::Instance().GetSystem<UDatabaseSystem>(); sys != nullptr) {
//         sys->PushTask([ret, pid = mOwner->GetFullID()](mysqlx::Schema &schema) {
//             for (const auto &[serializer, bExpired] : ret) {
//                 if (serializer == nullptr)
//                     continue;
//
//                 if (auto table = schema.getTable(serializer->GetTableName()); table.existsInDatabase()) {
//                     if (bExpired) {
//                         serializer->RemoveExpiredData(table, fmt::format("pid = {}", pid));
//                     }
//                     serializer->Execute(table);
//                 }
//                 delete serializer;
//             }
//             return true;
//         });
//     }
// }
//
// awaitable<void> UComponentModule::Deserialize() {
//     ADBQueryArray query;
//     std::string expr = fmt::format("pid = {}", GetOwner()->GetFullID());
//
//     for (const auto &ctx : std::views::values(mComponentMap)) {
//         for (const auto &name : ctx->GetTableList()) {
//             query.emplace_back(name, expr);
//         }
//     }
//
//     if (const auto sys = UGameWorld::Instance().GetSystem<UDatabaseSystem>(); sys != nullptr) {
//         if (const auto res = co_await sys->AsyncSelect(query, asio::use_awaitable); res != nullptr) {
//             for (const auto &ctx : std::views::values(mComponentMap)) {
//                 ctx->DeserializeComponent(*res);
//             }
//         }
//     }
// }

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
