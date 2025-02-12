#include "../include/connection_handler.h"
#include "../include/Connection.h"

IConnectionHandler::IConnectionHandler(Connection *conn)
    : mConn(conn) {
}

GameWorld * IConnectionHandler::GetWorld() const {
    return mConn->GetWorld();
}
