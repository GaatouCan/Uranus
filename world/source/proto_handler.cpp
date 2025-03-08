#include "../include/proto_handler.h"
#include "../include/proto_route.h"

IProtoHandler::IProtoHandler(UProtoRoute *route)
    : owner_(route) {

}

UProtoRoute * IProtoHandler::getOwner() const {
    return owner_;
}

UGameWorld * IProtoHandler::getWorld() const {
    return owner_->getWorld();
}



