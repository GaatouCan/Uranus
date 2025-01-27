#pragma once

#include "../ByteArray.h"

#include <functional>
#include <memory>
#include <string>


class BASE_API UReactor {

    std::shared_ptr<class UTaskQueue> mTaskQueue;

public:
    UReactor();
    virtual ~UReactor();

    DISABLE_COPY_MOVE(UReactor)

    void SetTaskQueue(const std::shared_ptr<UTaskQueue> &queue);

    [[nodiscard]] std::shared_ptr<UTaskQueue> GetTaskQueue() const;

    void PushTask(const std::function<void(UReactor *)> &task) const;
    void PushTask(std::function<void(UReactor *)> &&task) const;

    template<typename ReactorType, typename Functor, typename... ARGS>
    requires std::derived_from<ReactorType, UReactor>
    void PushTask(Functor &&func, ARGS &&... args) const {
        this->PushTask([this, func = std::forward<Functor>(func), ...args = std::forward<ARGS>(args)](UReactor *reactor) mutable {
            std::invoke(func, dynamic_cast<ReactorType *>(reactor), args...);
        });
    }

    virtual void Invoke(const std::string &func, FByteArray &&bytes) {}
};
