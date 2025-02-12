#pragma once

#include <impl/Package.h>


class IPlayerComponent {

    class IComponentContext *mContext;

public:
    explicit IPlayerComponent(IComponentContext *ctx);
    virtual ~IPlayerComponent();

    [[nodiscard]] virtual const char *GetComponentName() const = 0;

    IPlayerComponent() = delete;

    [[nodiscard]] IComponentContext *GetComponentContext() const;
    [[nodiscard]] class ComponentModule *GetModule() const;
    [[nodiscard]] class Player *GetOwner() const;
    [[nodiscard]] class GameWorld *GetWorld() const;

    virtual void OnLogin();
    virtual void OnLogout();
    virtual void OnDayChange(bool is_login);

    virtual void SyncCache(struct CacheNode *node);

    void Send(IPackage *pkg) const;

    void Send(int32_t id, std::string_view data) const;
    void Send(int32_t id, const std::stringstream &ss) const;
};
