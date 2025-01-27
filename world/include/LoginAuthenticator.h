#pragma once

#include "LoginHandler.h"

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

    template<typename T>
    requires std::derived_from<T, ILoginHandler>
    void SetHandler() {
        if (mHandler != nullptr) {
            mHandler.reset();
        }
        mHandler = std::make_unique<T>(this);
    }

private:
    UGameWorld* mWorld;
    std::unique_ptr<ILoginHandler> mHandler;
};
