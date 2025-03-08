#pragma once

#include "player_id.h"

#include <memory>
#include <string>
#include <asio/awaitable.hpp>


class UConnection;
class IBasePlayer;

using asio::awaitable;

struct BASE_API FLoginInfo {
    FPlayerID pid;
    std::string token;
};

class BASE_API ILoginHandler {

    class ULoginAuthenticator *owner_;

public:
    ILoginHandler() = delete;

    explicit ILoginHandler(ULoginAuthenticator *owner);
    virtual ~ILoginHandler() = default;

    [[nodiscard]] ULoginAuthenticator *getOwner() const;
    [[nodiscard]] class UGameWorld *getWorld() const;

    virtual awaitable<FLoginInfo> parseLoginInfo(class IPackage *) = 0;
    virtual awaitable<std::shared_ptr<IBasePlayer>> onPlayerLogin(const std::shared_ptr<UConnection>&, const FLoginInfo&) = 0;
};
