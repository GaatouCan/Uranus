#include "Handler.h"
#include "PacketCodec.h"
#include "LoginHandlerImpl.h"

#include <Recycler.h>
#include <Network/Connection.h>
#include <Login/LoginAuth.h>


UServerHandler::UServerHandler() {
}

UServerHandler::~UServerHandler() {
}

void UServerHandler::InitLoginAuth(ULoginAuth *auth) const {
    if (auth != nullptr) {
        auth->SetLoginHandler<ULoginHandler>();
    }
}

void UServerHandler::InitConnection(const std::shared_ptr<UConnection> &conn) const {
    if (conn != nullptr) {
        conn->SetPackageCodec<UPacketCodec>();
    }
}

std::shared_ptr<IRecycler> UServerHandler::CreatePackagePool(asio::io_context &ctx) {
    return std::make_shared<TRecycler<FPacket>>(ctx);
}
