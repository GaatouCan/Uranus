#include "../include/login_handler.h"
#include "../include/login_authenticator.h"


ILoginHandler::ILoginHandler(LoginAuthenticator *owner)
    : mOwner(owner) {

}

LoginAuthenticator * ILoginHandler::GetOwner() const {
    return mOwner;
}

GameWorld *ILoginHandler::GetWorld() const {
    return mOwner->GetWorld();
}

