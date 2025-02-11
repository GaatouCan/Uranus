#pragma once

#include "base_scene.h"


class BASE_API MainScene final : public IBaseScene {

    class PackagePool* mPool;

    asio::io_context mContext;
    ThreadID mThreadID;

public:
    MainScene(SceneManager *owner, int32_t id);
    ~MainScene() override;

    void SetThreadID(ThreadID tid);
    [[nodiscard]] ThreadID GetThreadID() const;

    asio::io_context& GetIOContext();
    [[nodiscard]] PackagePool* GetPackagePool() const;
};
