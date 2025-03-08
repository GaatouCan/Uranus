#pragma once

#include <login_handler.h>


class ULoginHandler final : public ILoginHandler {
public:
    explicit ULoginHandler(ULoginAuthenticator *owner);
    awaitable<std::shared_ptr<IBasePlayer>> onPlayerLogin(const std::shared_ptr<UConnection> &conn, const FLoginInfo &info) override;
    awaitable<FLoginInfo> parseLoginInfo(IPackage *) override;
};
