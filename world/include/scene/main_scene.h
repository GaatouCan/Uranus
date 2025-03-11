#pragma once

#include "base_scene.h"


class BASE_API UMainScene final : public IBaseScene {

    class IRecycler* pool_;

    asio::io_context context_;
    AThreadID thread_;

public:
    UMainScene(USceneManager *owner, int32_t id);
    ~UMainScene() override;

    void setThreadID(AThreadID tid);
    [[nodiscard]] AThreadID getThreadID() const;

    asio::io_context& getIOContext();
    [[nodiscard]] IRecycler* getPackagePool() const;
};
