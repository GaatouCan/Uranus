#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>


class UComponentModule;
class UPlayer;
class FSerializer;
class FDeserializer;


class IPlayerComponent {

    UComponentModule *mModule;

public:
    IPlayerComponent();
    virtual ~IPlayerComponent();

    void SetUpModule(UComponentModule *module);

    [[nodiscard]] UComponentModule *GetModule() const;
    [[nodiscard]] UPlayer *GetPlayer() const;
    [[nodiscard]] int64_t GetPlayerID() const;

    virtual void OnLogin();
    virtual void OnLogout();

    virtual void OnDayChange();

    [[nodiscard]] virtual constexpr std::vector<std::string> GetTableList() const {
        return {};
    }

    virtual void Serialize(const std::shared_ptr<FSerializer> &s);
    virtual void Deserialize(FDeserializer &ds);

    void SendToClient(uint32_t id, const std::string &data) const;
    void SendToService(int32_t sid, uint32_t id, const std::string &data) const;
    void SendToPlayer(int64_t pid, uint32_t id, const std::string &data) const;
};

#define SEND_TO_CLIENT(sender, proto_type, message) \
    (sender)->SendToClient(static_cast<uint32_t>(protocol::EProtoType::proto_type), (message).SerializeAsString());
