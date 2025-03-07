#pragma once

#include "base_scene.h"


class BASE_API UMainScene final : public IBaseScene {

    class UPackagePool* pool_;

    asio::io_context ctx_;
    AThreadID thread_;

public:
    UMainScene(USceneManager *owner, int32_t id);
    ~UMainScene() override;

    void SetThreadID(AThreadID tid);
    [[nodiscard]] AThreadID GetThreadID() const;

    asio::io_context& GetIOContext();
    [[nodiscard]] UPackagePool* GetPackagePool() const;
};
