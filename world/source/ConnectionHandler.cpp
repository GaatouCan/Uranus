#include "../include/ConnectionHandler.h"
#include "../include/Connection.h"

IConnectionHandler::IConnectionHandler(UConnection *conn)
    : mConn(conn) {
}

UGameWorld * IConnectionHandler::GetWorld() const {
    return mConn->GetWorld();
}
