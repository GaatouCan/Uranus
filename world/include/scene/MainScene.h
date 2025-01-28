#pragma once

#include "AbstractScene.h"


class BASE_API UMainScene final : public IAbstractScene {

    class UPackagePool* mPackagePool;

    asio::io_context mIOContext;
    AThreadID mThreadID;

public:
    UMainScene(USceneManager *owner, uint32_t id);
    ~UMainScene() override;

    void SetThreadID(AThreadID tid);
    [[nodiscard]] AThreadID GetThreadID() const;

    asio::io_context& GetIOContext();
    [[nodiscard]] UPackagePool* GetPackagePool() const;
};
