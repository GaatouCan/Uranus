#include "ComponentModule.h"
#include "player.h"

#include "Component/Knapsack/Knapsack.h"
#include "Component/Appear/Appear.h"


#include <Database/DataAccess.h>
#include <Database/Serializer.h>
#include <Database/Deserializer.h>


UComponentModule::UComponentModule(UPlayer *player)
    : player_(player) {

    CreateComponent<UKnapsack>();
    CreateComponent<UAppear>();
}

UComponentModule::~UComponentModule() {

}

UPlayer *UComponentModule::GetPlayer() const {
    return player_;
}

void UComponentModule::OnLogin() const {
    for (const auto &type: orderedVector_) {
        if (const auto iter = componentMap_.find(type); iter != componentMap_.end()) {
            iter->second->OnLogin();
        }
    }
}

void UComponentModule::OnLogout() const {
    for (const auto &type: orderedVector_) {
        if (const auto iter = componentMap_.find(type); iter != componentMap_.end()) {
            iter->second->OnLogout();
        }
    }
}

void UComponentModule::OnDayChange() const {
    for (const auto &type: orderedVector_) {
        if (const auto iter = componentMap_.find(type); iter != componentMap_.end()) {
            iter->second->OnDayChange();
        }
    }
}

void UComponentModule::Serialize() const {
    auto serializer = std::make_shared<FSerializer>();

    for (const auto &type: orderedVector_) {
        if (const auto iter = componentMap_.find(type); iter != componentMap_.end()) {
            iter->second->Serialize(serializer);
        }
    }

    auto *dataAccess = GetPlayer()->GetServer()->GetModule<UDataAccess>();
    if (dataAccess == nullptr)
        return;

    dataAccess->PushTransaction([serializer](mysqlx::Schema &schema) {
        serializer->Serialize(schema);
    });
}

awaitable<void> UComponentModule::Deserialize() {
    try {
        AQueryArray array;
        std::string expr = fmt::format("pid = {}", GetPlayer()->GetPlayerID());

        for (const auto &val : componentMap_ | std::views::values) {
            for (const auto &table : val->GetTableList()) {
                array.emplace_back(table, expr);
            }
        }

        auto *dataAccess = GetPlayer()->GetServer()->GetModule<UDataAccess>();
        if (dataAccess == nullptr)
            co_return;

        const auto res = co_await dataAccess->AsyncSelect(array, asio::use_awaitable);
        if (res == nullptr)
            co_return;

        FDeserializer deser(res);
        for (const auto &type: orderedVector_) {
            if (const auto iter = componentMap_.find(type); iter != componentMap_.end()) {
                iter->second->Deserialize(deser);
            }
        }

    } catch (const std::exception &e) {
        SPDLOG_ERROR("{}", e.what());
    }
}
