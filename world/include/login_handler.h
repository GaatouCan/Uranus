#pragma once

#include "player_id.h"

#include <memory>
#include <string>
#include <asio/awaitable.hpp>


class UConnection;
class IBasePlayer;

using asio::awaitable;

struct BASE_API LoginInfo {
    FPlayerID pid;
    std::string token;
};

class BASE_API ILoginHandler {

    class ULoginAuthenticator *owner_;

public:
    ILoginHandler() = delete;

    explicit ILoginHandler(ULoginAuthenticator *owner);
    virtual ~ILoginHandler() = default;

    [[nodiscard]] ULoginAuthenticator *GetOwner() const;
    [[nodiscard]] class UGameWorld *GetWorld() const;

    virtual awaitable<LoginInfo> ParseLoginInfo(class IPackage *) = 0;
    virtual awaitable<std::shared_ptr<IBasePlayer>> OnPlayerLogin(const std::shared_ptr<UConnection>&, const LoginInfo&) = 0;
};
