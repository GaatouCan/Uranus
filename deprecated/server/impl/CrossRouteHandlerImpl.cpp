#include "CrossRouteHandlerImpl.h"

#include <GameWorld.h>
#include <system/protocol/ProtocolSystem.h>

awaitable<void> UCrossRouteHandlerImpl::OnReadPackage(IPackage *package) {
    if (const auto sys = UGameWorld::Instance().GetSystem<UProtocolSystem>(); sys != nullptr) {
        co_await sys->OnCrossPackage(package);
    }
}
