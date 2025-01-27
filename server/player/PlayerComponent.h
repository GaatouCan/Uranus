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
    [[nodiscard]] class UComponentModule *GetModule() const;
    [[nodiscard]] class UPlayer *GetOwner() const;

    virtual void OnLogin();
    virtual void OnLogout();
    virtual void OnDayChange(bool bLogin);

    virtual void SyncCache(struct FCacheNode *node);

    void Send(IPackage *pkg) const;

    void Send(int32_t id, std::string_view data) const;
    void Send(int32_t id, const std::stringstream &ss) const;
};
