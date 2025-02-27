#include "../include/connection_handler.h"
#include "../include/connection.h"

IConnectionHandler::IConnectionHandler(const std::weak_ptr<Connection> &conn)
    : conn_(conn) {
}

GameWorld * IConnectionHandler::GetWorld() const {
    return conn_.lock()->GetWorld();
}
