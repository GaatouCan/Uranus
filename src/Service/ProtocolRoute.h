#pragma once

#include "Package.h"
#include <absl/container/flat_hash_map.h>


template<CPackageType Package, typename Functor>
class TProtocolRoute {

public:
    TProtocolRoute() = default;
    virtual ~TProtocolRoute() = default;

    DISABLE_COPY_MOVE(TProtocolRoute)

    void Register(const uint32_t id, const Functor &func) {
        mProtocolMap.insert_or_assign(id, func);
    }

    template<class ... Args>
    void OnReceivePackage(const std::shared_ptr<Package> &pkg, Args ... args) const {
        const auto iter = mProtocolMap.find(pkg->GetPackageID());
        if (iter == mProtocolMap.end())
            return;

        std::invoke(iter->second, pkg->GetPackageID(), pkg, std::forward<Args>(args)...);
    }

protected:
    absl::flat_hash_map<uint32_t, Functor> mProtocolMap;
};
