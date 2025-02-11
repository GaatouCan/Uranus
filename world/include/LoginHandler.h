#pragma once

#include "PlayerID.h"

#include <memory>
#include <string>
#include <asio/awaitable.hpp>


class Connection;
class IBasePlayer;

using asio::awaitable;

struct BASE_API FLoginInfo {
    FPlayerID pid;
    std::string token;
};

class BASE_API ILoginHandler {

    class LoginAuthenticator *owner_;

public:
    ILoginHandler() = delete;

    explicit ILoginHandler(LoginAuthenticator *owner);
    virtual ~ILoginHandler() = default;

    [[nodiscard]] LoginAuthenticator *GetOwner() const;
    [[nodiscard]] class GameWorld *GetWorld() const;

    virtual awaitable<FLoginInfo> ParseLoginInfo(class IPackage *) = 0;
    virtual awaitable<std::shared_ptr<IBasePlayer>> OnPlayerLogin(const std::shared_ptr<Connection>&, const FLoginInfo&) = 0;
};
