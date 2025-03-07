#pragma once

#include "base_command.h"

class BASE_API IOperateCommand : public IBaseCommand {

    int64_t op_id_;
    int64_t create_time_;
    int64_t update_time_;

    std::string creator_;

public:
    IOperateCommand(UCommandSystem *owner, UCommandObject object);

    void SetCommandID(int64_t id);
    void SetCreateTime(int64_t time);
    void SetUpdateTime(int64_t time);
    void SetCreator(const std::string &creator);

    [[nodiscard]] int64_t GetCommandID() const;
    [[nodiscard]] int64_t GetCreateTime() const;
    [[nodiscard]] int64_t GetUpdateTime() const;
    [[nodiscard]] std::string GetCreator() const;
};
