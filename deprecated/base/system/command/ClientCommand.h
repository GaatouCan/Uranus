#pragma once

#include "BaseCommand.h"


class IClientCommand : public IBaseCommand {

    uint32_t mSender;

public:
    explicit IClientCommand(UCommandObject param);

    void SetSender(uint32_t sender);
    [[nodiscard]] uint32_t GetSender() const;
};

