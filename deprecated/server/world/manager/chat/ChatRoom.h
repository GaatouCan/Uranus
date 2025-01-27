#pragma once

#include <common.h>
#include <memory>
#include <set>
#include <vector>
#include <asio.hpp>

#include <UniqueID.h>
#include <PlayerID.h>

using asio::awaitable;

class UPlayer;
struct FCacheNode;

struct FChatContent {
    long long clientTime;
    long long serverTime;

    uint32_t clientIndex;
    uint32_t serverIndex;

    std::string content;

    long long refTime;
    uint32_t refIndex;
};

class UChatRoom final : public std::enable_shared_from_this<UChatRoom> {

    class UChatManager *mOwner;

    FUniqueID mRoomId;
    FPlayerID mLeaderId;

    std::set<FPlayerID> mMemberSet;

    ATimePoint mCreateTime;
    ATimePoint mLastUpdateTime;

    uint32_t mChatIndex;

public:
    UChatRoom() = delete;

    explicit UChatRoom(UChatManager *owner);
    ~UChatRoom();

    UChatRoom &SetRoomID(FUniqueID roomId);
    [[nodiscard]] FUniqueID GetRoomID() const;

    UChatRoom &SetLeaderID(const FPlayerID &leaderId);
    [[nodiscard]] FPlayerID GetLeaderID() const;

    awaitable<void> SendAllRoomInfo(const std::shared_ptr<UPlayer> &plr = nullptr) const;

    awaitable<void> UpdateMemberInfo(std::set<FPlayerID> members, const std::vector<FCacheNode *> &cacheVec = {}) const;

    void OnChat(const std::shared_ptr<UPlayer> &plr, const FChatContent &content);
};