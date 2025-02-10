#include "../include/LoginHandler.h"
#include "../include/LoginAuthenticator.h"


ILoginHandler::ILoginHandler(ULoginAuthenticator *owner)
    : owner_(owner) {

}

ULoginAuthenticator * ILoginHandler::GetOwner() const {
    return owner_;
}

UGameWorld *ILoginHandler::GetWorld() const {
    return owner_->GetWorld();
}

