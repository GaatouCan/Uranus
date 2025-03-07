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

    void Init();
    [[nodiscard]] UGameWorld *GetWorld() const;

    bool VerifyAddress(const asio::ip::address &addr);

    FPlayerID VerifyToken(FPlayerID pid, const std::string &token);

    awaitable<void> OnLogin(const AConnectionPointer &conn, class IPackage *pkg);

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
    UGameWorld* world_;
    std::unique_ptr<ILoginHandler> handler_;
};
