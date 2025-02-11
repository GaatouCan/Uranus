#include "ComponentModule.h"
#include "Player.h"

// #include "../world/component/appearance/AppearanceCT.h"

#include <GameWorld.h>
// #include <system/database/DatabaseSystem.h>


UComponentModule::UComponentModule(UPlayer *plr)
    : mOwner(plr){

    // CreateComponent<UAppearanceCT>();
}

UComponentModule::~UComponentModule() {
    for (const auto &ctx : std::views::values(mComponentMap)) {
        delete ctx;
    }
}

UPlayer * UComponentModule::GetOwner() const {
    return mOwner;
}

void UComponentModule::OnDayChange() {
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

void UComponentModule::OnLogin() {
    for (const auto &ctx: std::views::values(mComponentMap)) {
        ctx->GetComponent()->OnLogin();
    }
}

void UComponentModule::OnLogout() {
    for (const auto &ctx: std::views::values(mComponentMap)) {
        ctx->GetComponent()->OnLogout();
    }
}

PlayerID UComponentModule::GetPlayerID() const {
    if (mOwner)
        return mOwner->GetPlayerID();
    return {};
}

void UComponentModule::SyncCache(FCacheNode *node) {
    for (const auto &ctx: std::views::values(mComponentMap)) {
        ctx->GetComponent()->SyncCache(node);
    }
}
