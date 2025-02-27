#include "../include/login_handler.h"
#include "../include/login_authenticator.h"


ILoginHandler::ILoginHandler(LoginAuthenticator *owner)
    : owner_(owner) {

}

LoginAuthenticator * ILoginHandler::GetOwner() const {
    return owner_;
}

GameWorld *ILoginHandler::GetWorld() const {
    return owner_->GetWorld();
}

