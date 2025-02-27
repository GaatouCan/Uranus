#include "../include/protocol_handler.h"
#include "../include/protocol_route.h"

IProtocolHandler::IProtocolHandler(ProtocolRoute *route)
    : owner_(route) {

}

ProtocolRoute * IProtocolHandler::GetOwner() const {
    return owner_;
}

GameWorld * IProtocolHandler::GetWorld() const {
    return owner_->GetWorld();
}



