#pragma once

#include "login_handler.h"

#include <asio.hpp>


class UConnection;
using AConnectionPointer = std::shared_ptr<UConnection>;


class ULoginAuthenticator final {

    friend class UGameWorld;

    explicit ULoginAuthenticator(UGameWorld *world);
    ~ULoginAuthenticator();

public:
    ULoginAuthenticator() = delete;

    DISABLE_COPY_MOVE(ULoginAuthenticator)

    void init();
    [[nodiscard]] UGameWorld *getWorld() const;

    bool verifyAddress(const asio::ip::address &addr);

    FPlayerID verifyToken(FPlayerID pid, const std::string &token);

    awaitable<void> onLogin(const AConnectionPointer &conn, class IPackage *pkg);

    template<typename T, typename... Args>
    requires std::derived_from<T, ILoginHandler>
    void setHandler(Args &&... args) {
        if (handler_ != nullptr) {
            handler_.reset();
        }
        handler_ = std::make_unique<T>(this, std::forward<Args>(args)...);
    }

    void abort() const;

private:
    UGameWorld* world_;
    std::unique_ptr<ILoginHandler> handler_;
};
