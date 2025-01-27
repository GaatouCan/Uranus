#pragma once

#include <LoginHandler.h>


class ULoginHandlerImpl final : public ILoginHandler {
public:
    explicit ULoginHandlerImpl(ULoginAuthenticator *owner);
    awaitable<std::shared_ptr<IBasePlayer>> OnPlayerLogin(const std::shared_ptr<UConnection> &conn, const FLoginInfo &info) override;
    awaitable<FLoginInfo> ParseLoginInfo(IPackage *) override;
};
