#pragma once

#include <login_handler.h>


class LoginHandler final : public ILoginHandler {
public:
    explicit LoginHandler(ULoginAuthenticator *owner);
    awaitable<std::shared_ptr<IBasePlayer>> OnPlayerLogin(const std::shared_ptr<UConnection> &conn, const LoginInfo &info) override;
    awaitable<LoginInfo> ParseLoginInfo(IPackage *) override;
};
