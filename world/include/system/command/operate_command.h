#pragma once

#include "base_command.h"

class BASE_API IOperateCommand : public IBaseCommand {

    int64_t operateID_;
    int64_t createTime_;
    int64_t updateTime_;

    std::string creator_;

public:
    IOperateCommand(UCommandSystem *owner, UCommandObject object);

    void setCommandID(int64_t id);
    void setCreateTime(int64_t time);
    void setUpdateTime(int64_t time);
    void setCreator(const std::string &creator);

    [[nodiscard]] int64_t getCommandID() const;
    [[nodiscard]] int64_t getCreateTime() const;
    [[nodiscard]] int64_t getUpdateTime() const;
    [[nodiscard]] std::string getCreator() const;
};
