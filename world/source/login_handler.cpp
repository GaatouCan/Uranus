#include "../include/login_handler.h"
#include "../include/login_authenticator.h"


ILoginHandler::ILoginHandler(ULoginAuthenticator *owner)
    : owner_(owner) {

}

ULoginAuthenticator * ILoginHandler::getOwner() const {
    return owner_;
}

UGameWorld *ILoginHandler::getWorld() const {
    return owner_->getWorld();
}

