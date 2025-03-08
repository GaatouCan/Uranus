#pragma once

#include <impl/package.h>


class IPlayerComponent {

    class ComponentModule *module_;

public:
    explicit IPlayerComponent(ComponentModule *module);
    virtual ~IPlayerComponent();

    IPlayerComponent() = delete;

    virtual void serialize(const std::shared_ptr<class USerializer> &s) {}
    virtual void deserialize(class UDeserializer &ds) {}

    [[nodiscard]] virtual constexpr std::vector<std::string> getTableList() const {
        return {};
    }

    [[nodiscard]] ComponentModule *getModule() const;
    [[nodiscard]] class UPlayer *getOwner() const;
    [[nodiscard]] class UGameWorld *getWorld() const;

    virtual void onLogin();
    virtual void onLogout();
    virtual void onDayChange(bool is_login);

    virtual void syncCache(struct CacheNode *node);

    void send(IPackage *pkg) const;

    void send(int32_t id, std::string_view data) const;
    void send(int32_t id, const std::stringstream &ss) const;
};
