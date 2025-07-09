#pragma once

#include <map>
#include <memory>

#include "../../Manager.h"
#include "PlayerCache.h"

class FPacket;

class UPlayerManager final : public IManager {

    using Super = IManager;

public:
    explicit UPlayerManager(UGameWorld *world);
    ~UPlayerManager() override;

    [[nodiscard]] constexpr const char *GetManagerName() const override {
        return "Player Manager";
    }

    void OnSyncPlayer(const std::shared_ptr<FPacket> &pkt);
    [[nodiscard]] const FPlayerCache *GetPlayerCache(int64_t pid) const;

private:
    std::map<int64_t, FPlayerCache> cacheMap_;
};
