#include "../include/connection_handler.h"
#include "../include/connection.h"

IConnectionHandler::IConnectionHandler(Connection *conn)
    : mConn(conn) {
}

GameWorld * IConnectionHandler::GetWorld() const {
    return mConn->GetWorld();
}
