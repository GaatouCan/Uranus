#pragma once

#include <impl/package.h>


class IPlayerComponent {

    class ComponentModule *module_;

public:
    explicit IPlayerComponent(ComponentModule *module);
    virtual ~IPlayerComponent();

    IPlayerComponent() = delete;

    virtual void Serialize(const std::shared_ptr<class USerializer> &s) {}
    virtual void Deserialize(class UDeserializer &ds) {}

    [[nodiscard]] virtual constexpr std::vector<std::string> GetTableList() const {
        return {};
    }

    [[nodiscard]] ComponentModule *GetModule() const;
    [[nodiscard]] class Player *GetOwner() const;
    [[nodiscard]] class UGameWorld *GetWorld() const;

    virtual void OnLogin();
    virtual void OnLogout();
    virtual void OnDayChange(bool is_login);

    virtual void SyncCache(struct CacheNode *node);

    void Send(IPackage *pkg) const;

    void Send(int32_t id, std::string_view data) const;
    void Send(int32_t id, const std::stringstream &ss) const;
};
