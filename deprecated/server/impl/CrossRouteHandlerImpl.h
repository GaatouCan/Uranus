#pragma once

#include <CrossRouteHandler.h>


class UCrossRouteHandlerImpl final : public ICrossRouteHandler {
public:
    awaitable<void> OnReadPackage(IPackage *) override;
};

