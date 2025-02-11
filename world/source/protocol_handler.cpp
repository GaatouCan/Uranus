#include "../include/ProtocolHandler.h"
#include "../include/ProtocolRoute.h"

IProtocolHandler::IProtocolHandler(ProtocolRoute *route)
    : owner_(route) {

}

ProtocolRoute * IProtocolHandler::GetOwner() const {
    return owner_;
}

GameWorld * IProtocolHandler::GetWorld() const {
    return owner_->GetWorld();
}



