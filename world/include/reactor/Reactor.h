#pragma once

#include "../byte_array.h"

#include <functional>
#include <memory>
#include <string>


class BASE_API IReactor {

    std::shared_ptr<class TaskQueue> queue_;

public:
    IReactor() = default;
    virtual ~IReactor();

    DISABLE_COPY_MOVE(IReactor)

    void SetTaskQueue(const std::shared_ptr<TaskQueue> &queue);

    [[nodiscard]] std::shared_ptr<TaskQueue> GetTaskQueue() const;

    void PushTask(const std::function<void(IReactor *)> &task) const;
    void PushTask(std::function<void(IReactor *)> &&task) const;

    template<typename ReactorType, typename Functor, typename... Args>
    requires std::derived_from<ReactorType, IReactor>
    void PushTask(Functor &&func, Args &&... args) const {
        this->PushTask([this, func = std::forward<Functor>(func), ...args = std::forward<Args>(args)](IReactor *reactor) mutable {
            std::invoke(func, dynamic_cast<ReactorType *>(reactor), args...);
        });
    }

    virtual void Invoke(const std::string &func, ByteArray &&bytes) {}
};
