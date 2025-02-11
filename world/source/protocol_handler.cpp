#include "../include/protocol_handler.h"
#include "../include/protocol_route.h"

IProtocolHandler::IProtocolHandler(ProtocolRoute *route)
    : mOwner(route) {

}

ProtocolRoute * IProtocolHandler::GetOwner() const {
    return mOwner;
}

GameWorld * IProtocolHandler::GetWorld() const {
    return mOwner->GetWorld();
}



