#pragma once

#include <system/manager/BaseManager.h>
#include <PlayerID.h>

#include <mutex>
#include <shared_mutex>

struct FFriendNode {
    FPlayerID leftPlayer;
    FPlayerID rightPlayer;
    ATimePoint timestamp;
};

struct FFriendApply {
    FPlayerID from;
    FPlayerID to;
    ATimePoint timestamp;
};

struct FFriendBlack {
    FPlayerID black;
    FPlayerID blacked;
    ATimePoint timestamp;
};

using AFriendMap = std::unordered_map<FPlayerID, FFriendNode, FPlayerHash>;

class UFriendManager final : public IBaseManager {

    std::unordered_map<FPlayerID, AFriendMap, FPlayerHash> mFriendMap;
    std::mutex mMutex;
    mutable std::shared_mutex mSharedMutex;

    std::unordered_map<FPlayerID, std::unordered_map<FPlayerID, FFriendApply, FPlayerHash>, FPlayerHash> mApplyMap;

public:
    void Init() override;

    MANAGER_BODY(UFriendManager)

    bool IsFriend(const FPlayerID &lhs, const FPlayerID &rhs) const;

    bool IsInBlackList(const FPlayerID &lhs, const FPlayerID &rhs) const;
    int RemoveBlackList(const FPlayerID &lhs, const FPlayerID &rhs);

    int SendApply(const FPlayerID &lhs, const FPlayerID &rhs);
    int RemoveApply(const FPlayerID &lhs, const FPlayerID &rhs);

    int AddFriend(const FPlayerID &lhs, const FPlayerID &rhs);
};

