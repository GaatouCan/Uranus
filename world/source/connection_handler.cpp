#include "../include/connection_handler.h"
#include "../include/Connection.h"

IConnectionHandler::IConnectionHandler(Connection *conn)
    : conn_(conn) {
}

GameWorld * IConnectionHandler::GetWorld() const {
    return conn_->GetWorld();
}
