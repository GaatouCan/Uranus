#pragma once

#include "AbstractScene.h"


class BASE_API UMainScene final : public IAbstractScene {

    class UPackagePool* pool_;

    asio::io_context ctx_;
    AThreadID tid_;

public:
    UMainScene(USceneManager *owner, int32_t id);
    ~UMainScene() override;

    void SetThreadID(AThreadID tid);
    [[nodiscard]] AThreadID GetThreadID() const;

    asio::io_context& GetIOContext();
    [[nodiscard]] UPackagePool* GetPackagePool() const;
};
