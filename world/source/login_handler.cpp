#include "../include/login_handler.h"
#include "../include/login_authenticator.h"


ILoginHandler::ILoginHandler(ULoginAuthenticator *owner)
    : owner_(owner) {

}

ULoginAuthenticator * ILoginHandler::GetOwner() const {
    return owner_;
}

UGameWorld *ILoginHandler::GetWorld() const {
    return owner_->GetWorld();
}

