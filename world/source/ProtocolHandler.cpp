#include "../include/ProtocolHandler.h"
#include "../include/ProtocolRoute.h"

IProtocolHandler::IProtocolHandler(UProtocolRoute *route)
    : mOwner(route) {

}

UProtocolRoute * IProtocolHandler::GetOwner() const {
    return mOwner;
}

UGameWorld * IProtocolHandler::GetWorld() const {
    return mOwner->GetWorld();
}



