#pragma once

#include "base_scene.h"


class BASE_API MainScene final : public IBaseScene {

    class PackagePool* pool_;

    asio::io_context ctx_;
    ThreadID thread_;

public:
    MainScene(SceneManager *owner, int32_t id);
    ~MainScene() override;

    void SetThreadID(ThreadID tid);
    [[nodiscard]] ThreadID GetThreadID() const;

    asio::io_context& GetIOContext();
    [[nodiscard]] PackagePool* GetPackagePool() const;
};
