#include "../include/ConnectionHandler.h"
#include "../include/Connection.h"

IConnectionHandler::IConnectionHandler(UConnection *conn)
    : conn_(conn) {
}

UGameWorld * IConnectionHandler::GetWorld() const {
    return conn_->GetWorld();
}
