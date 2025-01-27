#include "ChatRoom.h"

#include "../../../player/Player.h"
#include "../../../player/PlayerManager.h"
#include "../../../common/ProtoType.h"
#include "ChatManager.h"

#include <GameWorld.h>
#include <utils.h>
#include <system/manager/ManagerSystem.h>

#include <chat.pb.h>

UChatRoom::UChatRoom(UChatManager *owner)
    : mOwner(owner),
      mRoomId(),
      mLeaderId(0),
      mChatIndex(1) {
}

UChatRoom::~UChatRoom() {
}

UChatRoom &UChatRoom::SetRoomID(const FUniqueID roomId) {
    mRoomId = roomId;
    return *this;
}

UChatRoom &UChatRoom::SetLeaderID(const FPlayerID &leaderId) {
    mLeaderId = leaderId;
    return *this;
}

FUniqueID UChatRoom::GetRoomID() const {
    return mRoomId;
}

FPlayerID UChatRoom::GetLeaderID() const {
    return mLeaderId;
}


awaitable<void> UChatRoom::SendAllRoomInfo(const std::shared_ptr<UPlayer> &plr) const {
    const auto plrMgr = UPlayerManager::Instance();
    if (plrMgr == nullptr)
        co_return;

    Chat::W2C_ChatRoomResponse response;

    response.set_roomid(mRoomId.ToString());
    response.set_leader(mLeaderId.ToUInt64());
    response.set_reason(Chat::W2C_ChatRoomResponse::NORMAL_SEND);

    for (const auto &memberId: mMemberSet) {
        auto node = co_await plrMgr->FindCacheNode(memberId);
        if (!node.has_value())
            continue;

        auto cacheNode = node.value();
        const auto member = response.add_memberlist();
        member->set_pid(memberId.ToUInt64());
        member->set_online(cacheNode.IsOnline());
    }

    if (plr != nullptr) {
        SEND_PACKAGE(plr,   W2C_ChatRoomResponse, response)
    } else {
        SEND_TO_PLAYER_SET(plrMgr, mMemberSet, W2C_ChatRoomResponse, response)
    }
}

awaitable<void> UChatRoom::UpdateMemberInfo(std::set<FPlayerID> members, const std::vector<FCacheNode *> &cacheVec) const {
    const auto plrMgr = UPlayerManager::Instance();
    if (plrMgr == nullptr)
        co_return;

    Chat::W2C_ChatRoomResponse response;

    response.set_roomid(mRoomId.ToString());
    response.set_leader(mLeaderId.ToUInt64());
    response.set_reason(Chat::W2C_ChatRoomResponse::MEMBER_UPDATE);

    for (const auto &cache: cacheVec) {
        if (cache == nullptr)
            continue;

        if (!mMemberSet.contains(cache->pid))
            continue;

        const auto member = response.add_memberlist();
        member->set_pid(cache->pid.ToUInt64());
        member->set_online(cache->IsOnline());

        members.erase(cache->pid);
    }

    for (const auto &memberId: members) {
        auto node = co_await plrMgr->FindCacheNode(memberId);
        if (!node.has_value())
            continue;

        auto cacheNode = node.value();
        const auto member = response.add_memberlist();
        member->set_pid(memberId.ToUInt64());
        member->set_online(cacheNode.IsOnline());
    }

    SEND_TO_PLAYER_SET(plrMgr, mMemberSet, W2C_ChatRoomResponse, response)
}

void UChatRoom::OnChat(const std::shared_ptr<UPlayer> &plr, const FChatContent &content) {
    Chat::W2C_ChatToRoomResponse response;

    response.set_roomid(mRoomId.ToString());
    response.set_clienttime(content.clientTime);
    response.set_clientindex(content.clientIndex);

    if (!mMemberSet.contains(plr->GetPlayerID())) {
        response.set_reason(Chat::W2C_ChatToRoomResponse::NOT_ROOM_MEMBER);
        SEND_PACKAGE(plr, W2C_ChatToRoomResponse, response)
        return;
    }

    if (const auto now = NowTimePoint(); mLastUpdateTime != now) {
        mLastUpdateTime = now;
        mChatIndex = 1;
    }

    response.set_servertime(utils::ToUnixTime(mLastUpdateTime));
    response.set_serverindex(mChatIndex);

    const auto plrMgr = UPlayerManager::Instance();
    if (plrMgr == nullptr) {
        response.set_reason(Chat::W2C_ChatToRoomResponse::IN_LIMITED);
        SEND_PACKAGE(plr, W2C_ChatRoomResponse, response)
        return;
    }

    response.set_reason(Chat::W2C_ChatToRoomResponse::SUCCESS);
    SEND_PACKAGE(plr, W2C_ChatToRoomResponse, response)

    Chat::W2C_OnChatRoomResponse msg;

    msg.set_roomid(mRoomId.ToString());
    msg.set_sender(plr->GetFullID());
    msg.set_content(content.content);
    msg.set_servertime(utils::ToUnixTime(mLastUpdateTime));
    msg.set_serverindex(mChatIndex++);
    msg.set_reftime(content.refTime);
    msg.set_refindex(content.refIndex);

    SEND_TO_PLAYER_SET(plrMgr, mMemberSet, W2C_OnChatRoomResponse, msg)
}
