#pragma once

#include <login_handler.h>


class LoginHandler final : public ILoginHandler {
public:
    explicit LoginHandler(ULoginAuthenticator *owner);
    awaitable<std::shared_ptr<IBasePlayer>> onPlayerLogin(const std::shared_ptr<UConnection> &conn, const FLoginInfo &info) override;
    awaitable<FLoginInfo> parseLoginInfo(IPackage *) override;
};
