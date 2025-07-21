#include "LoginHandler.h"
#include "LoginAuth.h"

ILoginHandler::ILoginHandler(ULoginAuth *module)
    : loginAuth_(module) {
}

ILoginHandler::~ILoginHandler() {
}

UServer *ILoginHandler::GetServer() const {
    return loginAuth_->GetServer();
}
