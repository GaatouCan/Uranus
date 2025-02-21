#pragma once

#include "base_command.h"

class BASE_API IOperateCommand : public IBaseCommand {

    int64_t mOperateID;
    int64_t mCreateTime;
    int64_t mUpdateTime;

    std::string mCreator;

public:
    IOperateCommand(CommandSystem *owner, CommandObject object);

    void SetCommandID(int64_t id);
    void SetCreateTime(int64_t time);
    void SetUpdateTime(int64_t time);
    void SetCreator(const std::string &creator);

    [[nodiscard]] int64_t GetCommandID() const;
    [[nodiscard]] int64_t GetCreateTime() const;
    [[nodiscard]] int64_t GetUpdateTime() const;
    [[nodiscard]] std::string GetCreator() const;
};
