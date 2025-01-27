#include "../include/LoginHandler.h"
#include "../include/LoginAuthenticator.h"


ILoginHandler::ILoginHandler(ULoginAuthenticator *owner)
    : mOwner(owner) {

}

ULoginAuthenticator * ILoginHandler::GetOwner() const {
    return mOwner;
}

UGameWorld *ILoginHandler::GetWorld() const {
    return mOwner->GetWorld();
}

