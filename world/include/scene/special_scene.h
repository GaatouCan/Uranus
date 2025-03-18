#pragma once

#include "base_scene.h"
#include "object.h"

#include <vector>
#include <concepts>

class BASE_API UGatherObject final : public UObject {

    uint32_t id_;
    uint32_t num_;

public:
    explicit UGatherObject(uint32_t id, uint32_t num = 1);
    ~UGatherObject() override;

    [[nodiscard]] uint32_t getGatherID() const;
    [[nodiscard]] uint32_t getNum() const;
};


class BASE_API ISpecialScene : public IBaseScene {

    std::vector<UObject *> objectList_;

public:
    ISpecialScene(USceneManager *owner, int32_t id);
    ~ISpecialScene() override;

    virtual void onGatherObject(const std::shared_ptr<IBasePlayer> &plr, UGatherObject *obj);

    void createGatherObject(std::map<uint32_t,uint32_t> gatherList);

protected:
    template<typename Type, typename... Args>
    Type *createSceneObject(Args && ... args) {
        const auto obj = new Type(std::forward<Args>(args)...);
        objectList_.emplace_back(obj);
        return obj;
    }
};
