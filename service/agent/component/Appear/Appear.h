#pragma once

#include "../../PlayerComponent.h"

#include <absl/container/flat_hash_map.h>
#include <Appear.orm.h>


class UAppear final : public IPlayerComponent {

    orm::FEntity_Appear appear_;
    absl::flat_hash_map<int, orm::FEntity_Avatar> avatarMap_;

public:
    UAppear();

    void ActiveAvatar(int index);
    void UseAvatar(int index);

    void SendInfo() const;
};

