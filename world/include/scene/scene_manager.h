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

    int32_t GenerateSceneID();

    void EmplaceScene(IBaseScene *scene);
    void CollectScene(ATimePoint time);

public:
    USceneManager() = delete;

    DISABLE_COPY_MOVE(USceneManager)

    void Init();

    UGameWorld *GetWorld() const;
    IBaseScene *GetNextMainScene();

    IBaseScene *GetScene(int32_t sid) const;

    template<typename T, typename... Args>
    requires std::derived_from<T, IBaseScene>
    T *CreateScene(Args &&... args) {
        const auto sid = GenerateSceneID();
        if (sid <= NORMAL_SCENE_ID_BEGIN || sid >= NORMAL_SCENE_ID_END)
            return nullptr;

        auto *scene = new T(this, sid, std::forward<Args>(args)...);
        EmplaceScene(scene);

        return scene;
    }

private:
    UGameWorld *world_;

    std::vector<IBaseScene *> main_scene_list_;
    std::vector<asio::io_context::work> work_list_;
    std::vector<std::thread> thread_list_;

    std::atomic_size_t next_main_idx_;

    std::unordered_map<int32_t, IBaseScene *> scene_map_;
    std::atomic_int32_t next_scene_id_;
    mutable std::shared_mutex scene_mtx_;

    ASystemTimer tick_timer_;
    std::atomic_bool running_;
};
