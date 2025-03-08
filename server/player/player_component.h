#pragma once

#include <impl/package.h>


class IPlayerComponent {

    class UComponentModule *module_;

public:
    explicit IPlayerComponent(UComponentModule *module);
    virtual ~IPlayerComponent();

    IPlayerComponent() = delete;

    virtual void serialize(const std::shared_ptr<class USerializer> &s) {}
    virtual void deserialize(class UDeserializer &ds) {}

    [[nodiscard]] virtual constexpr std::vector<std::string> getTableList() const {
        return {};
    }

    [[nodiscard]] UComponentModule *getModule() const;
    [[nodiscard]] class UPlayer *getOwner() const;
    [[nodiscard]] class UGameWorld *getWorld() const;

    virtual void onLogin();
    virtual void onLogout();
    virtual void onDayChange(bool is_login);

    virtual void syncCache(struct FCacheNode *node);

    void send(IPackage *pkg) const;

    void send(int32_t id, std::string_view data) const;
    void send(int32_t id, const std::stringstream &ss) const;
};
