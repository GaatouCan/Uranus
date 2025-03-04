#pragma once

#include "login_handler.h"

#include <asio.hpp>


class Connection;
using ConnectionPointer = std::shared_ptr<Connection>;


class LoginAuthenticator final {

    friend class GameWorld;

    explicit LoginAuthenticator(GameWorld *world);
    ~LoginAuthenticator();

public:
    LoginAuthenticator() = delete;

    DISABLE_COPY_MOVE(LoginAuthenticator)

    void Init();
    [[nodiscard]] GameWorld *GetWorld() const;

    bool VerifyAddress(const asio::ip::address &addr);

    PlayerID VerifyToken(PlayerID pid, const std::string &token);

    awaitable<void> OnLogin(const ConnectionPointer &conn, class IPackage *pkg);

    template<typename T, typename... Args>
    requires std::derived_from<T, ILoginHandler>
    void SetHandler(Args &&... args) {
        if (handler_ != nullptr) {
            handler_.reset();
        }
        handler_ = std::make_unique<T>(this, std::forward<Args>(args)...);
    }

    void AbortHandler() const;

private:
    GameWorld* world_;
    std::unique_ptr<ILoginHandler> handler_;
};
