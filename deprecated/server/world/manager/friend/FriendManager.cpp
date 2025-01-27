#include "FriendManager.h"

#include <GameWorld.h>
#include <system/manager/ManagerSystem.h>

#include <spdlog/spdlog.h>

void UFriendManager::Init() {
}

MANAGER_IMPL(UFriendManager)

bool UFriendManager::IsFriend(const FPlayerID &lhs, const FPlayerID &rhs) const {
    if (lhs.crossID != UGameWorld::Instance().GetServerID() || rhs.crossID != UGameWorld::Instance().GetServerID()) {
        return false;
    }

    std::shared_lock lock(mSharedMutex);

    auto iter = mFriendMap.find(lhs);
    if (iter == mFriendMap.end()) {
        iter = mFriendMap.find(rhs);
        if (iter == mFriendMap.end())
            return false;

        const auto iter2 = iter->second.find(lhs);
        if (iter2 == iter->second.end())
            return false;

        return iter2->second.timestamp >= NowTimePoint();
    }

    const auto iter2 = iter->second.find(rhs);
    if (iter2 == iter->second.end())
        return false;

    return iter2->second.timestamp >= NowTimePoint();
}

bool UFriendManager::IsInBlackList(const FPlayerID &lhs, const FPlayerID &rhs) const {
    // TODO
    return false;
}

int UFriendManager::RemoveBlackList(const FPlayerID &lhs, const FPlayerID &rhs) {
    if (!IsFriend(lhs, rhs))
        return 0;

    // TODO
    return 0;
}

int UFriendManager::SendApply(const FPlayerID &lhs, const FPlayerID &rhs) {
    if (IsFriend(lhs, rhs))
        return 1;

    if (IsInBlackList(rhs, lhs))
        return 2;

    if (const auto iter = mApplyMap.find(lhs); iter != mApplyMap.end()) {
        if (iter->second.contains(rhs)) {
            return 3;
        }
    } else {
        mApplyMap[lhs] = {};
    }

    mApplyMap[lhs][rhs] = {
        lhs, rhs, NowTimePoint()
    };
    return 0;
}

int UFriendManager::RemoveApply(const FPlayerID &lhs, const FPlayerID &rhs) {
    if (const auto iter = mApplyMap.find(lhs); iter != mApplyMap.end()) {
        if (iter->second.contains(rhs)) {
            iter->second.erase(rhs);
        }
    }
    return 0;
}

int UFriendManager::AddFriend(const FPlayerID &lhs, const FPlayerID &rhs) {
    if (IsFriend(lhs, rhs)) {
        return 0;
    }

    // 互相移除黑名单
    if (RemoveBlackList(lhs, rhs) != 0)
        return 1;

    if (RemoveBlackList(rhs, lhs) != 0)
        return 2;

    RemoveApply(lhs, rhs);

    std::scoped_lock lock(mMutex);
    if (!mFriendMap.contains(lhs)) {
        mFriendMap[lhs] = {};
    }

    mFriendMap[lhs][rhs] = {
        lhs, rhs, NowTimePoint()
    };
    return 0;
}
