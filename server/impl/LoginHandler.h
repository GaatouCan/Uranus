#pragma once

#include <LoginHandler.h>


class LoginHandler final : public ILoginHandler {
public:
    explicit LoginHandler(LoginAuthenticator *owner);
    awaitable<std::shared_ptr<IBasePlayer>> OnPlayerLogin(const std::shared_ptr<Connection> &conn, const LoginInfo &info) override;
    awaitable<LoginInfo> ParseLoginInfo(IPackage *) override;
};
