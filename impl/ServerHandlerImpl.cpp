#include "ServerHandlerImpl.h"
#include "LoginHandlerImpl.h"

#include <Recycler.h>
#include <Network/Connection.h>
#include <Login/LoginAuth.h>


void UServerHandler::InitLoginAuth(ULoginAuth *auth) const {
    if (auth != nullptr) {
        auth->SetLoginHandler<ULoginHandler>();
    }
}

void UServerHandler::InitConnection(const std::shared_ptr<UConnection> &conn) const {

}

std::shared_ptr<IRecycler> UServerHandler::CreatePackagePool(asio::io_context &ctx) {
    return nullptr;
}
