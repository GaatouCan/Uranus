#pragma once

#include <asio.hpp>
#include <atomic>
#include <shared_mutex>
#include <thread>
#include <concepts>

#include "base_scene.h"
#include "../utils.h"


static constexpr int32_t NORMAL_SCENE_ID_BEGIN = 1000;
static constexpr int32_t NORMAL_SCENE_ID_END = 99999;

class BASE_API USceneManager final
{
    friend class UGameWorld;

    explicit USceneManager(UGameWorld *world);
    ~USceneManager();

    int32_t generateSceneID();

    void emplaceScene(IBaseScene *scene);
    void collectScene(ATimePoint time);

public:
    USceneManager() = delete;

    DISABLE_COPY_MOVE(USceneManager)

    void init();

    UGameWorld *getWorld() const;
    IBaseScene *getNextMainScene();

    IBaseScene *getScene(int32_t sid) const;

    template<typename T, typename... Args>
    requires std::derived_from<T, IBaseScene>
    T *createScene(Args &&... args) {
        const auto sid = generateSceneID();
        if (sid <= NORMAL_SCENE_ID_BEGIN || sid >= NORMAL_SCENE_ID_END)
            return nullptr;

        auto *scene = new T(this, sid, std::forward<Args>(args)...);
        emplaceScene(scene);

        return scene;
    }

private:
    UGameWorld *world_;

    std::vector<IBaseScene *> mainSceneList_;
    std::vector<asio::io_context::work> workList_;
    std::vector<std::thread> threads_;

    std::atomic_size_t nextMainIndex_;

    absl::flat_hash_map<int32_t, IBaseScene *> sceneMap_;
    std::atomic_int32_t nextSceneID_;
    mutable std::shared_mutex sceneMutex_;

    ASystemTimer tickTimer_;
    std::atomic_bool running_;
};
