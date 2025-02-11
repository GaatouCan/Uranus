#include "ComponentModule.h"
#include "Player.h"

// #include "../world/component/appearance/AppearanceCT.h"

#include <GameWorld.h>
// #include <system/database/DatabaseSystem.h>


ComponentModule::ComponentModule(Player *plr)
    : owner_(plr){

    // CreateComponent<UAppearanceCT>();
}

ComponentModule::~ComponentModule() {
    for (const auto &ctx : std::views::values(component_map_)) {
        delete ctx;
    }
}

Player * ComponentModule::GetOwner() const {
    return owner_;
}

void ComponentModule::OnDayChange() {
    for (const auto &ctx : component_map_ | std::views::values) {
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
    for (const auto &ctx: std::views::values(component_map_)) {
        ctx->GetComponent()->OnLogin();
    }
}

void ComponentModule::OnLogout() {
    for (const auto &ctx: std::views::values(component_map_)) {
        ctx->GetComponent()->OnLogout();
    }
}

PlayerID ComponentModule::GetPlayerID() const {
    if (owner_)
        return owner_->GetPlayerID();
    return {};
}

void ComponentModule::SyncCache(CacheNode *node) {
    for (const auto &ctx: std::views::values(component_map_)) {
        ctx->GetComponent()->SyncCache(node);
    }
}
