#include "../include/connection_handler.h"
#include "../include/connection.h"

IConnectionHandler::IConnectionHandler(const std::weak_ptr<UConnection> &conn)
    : conn_(conn) {
}

UGameWorld * IConnectionHandler::GetWorld() const {
    return conn_.lock()->GetWorld();
}
