#include "../include/ProtocolHandler.h"
#include "../include/ProtocolRoute.h"

IProtocolHandler::IProtocolHandler(UProtocolRoute *route)
    : owner_(route) {

}

UProtocolRoute * IProtocolHandler::GetOwner() const {
    return owner_;
}

UGameWorld * IProtocolHandler::GetWorld() const {
    return owner_->GetWorld();
}



