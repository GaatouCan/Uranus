#include "Handler.h"
#include "PacketCodec.h"
#include "LoginHandler.h"

#include <Recycler.h>
#include <Network/Connection.h>
#include <Login/LoginAuth.h>


UServerHandler::UServerHandler() {
}

UServerHandler::~UServerHandler() {
}

void UServerHandler::InitLoginAuth(ULoginAuth *auth) {
    if (auth != nullptr) {
        auth->SetLoginHandler<ULoginHandler>();
    }
}

void UServerHandler::InitConnection(const std::shared_ptr<UConnection> &conn) {
    if (conn != nullptr) {
        conn->SetPackageCodec<UPacketCodec>();
    }
}

std::shared_ptr<IRecycler> UServerHandler::CreatePackagePool(asio::io_context &ctx) {
    return std::make_shared<TRecycler<FPacket>>(ctx);
}
