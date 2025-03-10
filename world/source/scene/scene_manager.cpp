#include "../../include/scene/scene_manager.h"
#include "../../include/scene/main_scene.h"
#include "../../include/game_world.h"
#include "../../include/server_logic.h"

#include <ranges>
#include <spdlog/spdlog.h>

USceneManager::USceneManager(UGameWorld *world)
    : world_(world),
      nextMainIndex_(0),
      nextSceneID_(NORMAL_SCENE_ID_BEGIN + 1),
      tickTimer_(world_->getIOContext()),
      running_(false) {
}

USceneManager::~USceneManager() {
    running_ = false;

    for (const auto it: sceneMap_ | std::views::values)
        delete it;

    workList_.clear();
    for (const auto it: mainSceneList_)
        delete it;

    for (auto &th: threads_) {
        if (th.joinable())
            th.join();
    }
}

int32_t USceneManager::generateSceneID() {
    int32_t id;

    std::shared_lock lock(sceneMutex_);
    if (sceneMap_.size() >= (NORMAL_SCENE_ID_END - NORMAL_SCENE_ID_BEGIN - 1))
        return -1;

    do {
        if (nextSceneID_ >= NORMAL_SCENE_ID_END)
            nextSceneID_ = NORMAL_SCENE_ID_BEGIN + 1;
        id = nextSceneID_++;
    } while (sceneMap_.contains(id));

    return id;
}

void USceneManager::emplaceScene(IBaseScene *scene) {
    if (scene == nullptr)
        return;

    std::unique_lock lock(sceneMutex_);
    sceneMap_[scene->getSceneID()] = scene;
}

void USceneManager::collectScene(const ATimePoint time) {
    std::unique_lock lock(sceneMutex_);
    constexpr auto zero_time_point = ATimePoint();

    for (auto it = sceneMap_.begin(); it != sceneMap_.end();) {
        if (it->second->destroyTimePoint_ > zero_time_point && it->second->destroyTimePoint_ < time && it->second->canDestroy()) {
            const auto *scene = it->second;
            it = sceneMap_.erase(it);
            delete scene;
            continue;
        }
        ++it;
    }
}

void USceneManager::init() {
    const auto &cfg = world_->getServerConfig();
    const auto num = cfg["server"]["io_thread"].as<int32_t>();

    const auto server = getWorld()->getServerLogic();
    // assert(server != nullptr);

    for (int32_t idx = 0; idx < num; ++idx) {
        auto scene = new UMainScene(this, idx, server->createPackagePool());

        workList_.emplace_back(scene->getIOContext());
        threads_.emplace_back([this, scene] {
            asio::signal_set signals(scene->getIOContext(), SIGINT, SIGTERM);
            signals.async_wait([scene](auto, auto) {
                scene->getIOContext().stop();
                spdlog::info("Main Scene[{}] Shutdown.", scene->getSceneID());
            });

            scene->setThreadID(std::this_thread::get_id());
            scene->getIOContext().run();
        });

        mainSceneList_.emplace_back(scene);
    }

    running_ = true;
    spdlog::info("Started With {} Thread(s).", num);

    co_spawn(getWorld()->getIOContext(), [this]() mutable -> awaitable<void> {
        try {
            auto point = NowTimePoint() + std::chrono::seconds(1);
            while (running_) {
                tickTimer_.expires_at(point);
                co_await tickTimer_.async_wait();

                if (running_)
                    collectScene(point);

                point += std::chrono::seconds(1);
            }
        } catch (const std::exception &e) {
            spdlog::error("SceneManager::Init() - Failed to run CollectScene {}", e.what());
        }
    }, asio::detached);
}

UGameWorld *USceneManager::getWorld() const {
    return world_;
}

IBaseScene *USceneManager::getNextMainScene() {
    if (mainSceneList_.empty())
        throw std::runtime_error("No context node available");

    const auto res = mainSceneList_[nextMainIndex_++];
    nextMainIndex_ = nextMainIndex_ % mainSceneList_.size();

    return res;
}

IBaseScene *USceneManager::getScene(const int32_t sid) const {
    if (sid < 0)
        return nullptr;

    if (sid < NORMAL_SCENE_ID_BEGIN) {
        if (sid >= mainSceneList_.size())
            return nullptr;

        return mainSceneList_[sid];
    }

    if (sid <= NORMAL_SCENE_ID_BEGIN || sid >= NORMAL_SCENE_ID_END)
        return nullptr;

    std::shared_lock lock(sceneMutex_);
    if (const auto it = sceneMap_.find(sid); it != sceneMap_.end())
        return it->second;

    return nullptr;
}
