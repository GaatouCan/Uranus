#pragma once

#include "BaseCommand.h"


class IOperateCommand : public IBaseCommand {

    uint64_t mCommandID;

    uint64_t mCreateTime;
    uint64_t mUpdateTime;

    std::string mCreator;

public:
    explicit IOperateCommand(UCommandObject param);

    void SetCommandID(uint64_t id);
    void SetCreateTime(uint64_t time);
    void SetCreator(const std::string &creator);

    [[nodiscard]] uint64_t GetCommandID() const;
    [[nodiscard]] uint64_t GetCreateTime() const;
    [[nodiscard]] uint64_t GetUpdateTime() const;
    [[nodiscard]] std::string GetCreator() const;
};

