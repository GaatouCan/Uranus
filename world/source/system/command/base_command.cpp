//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/base_command.h"

IBaseCommand::IBaseCommand(CommandObject object)
    : mObject(std::move(object)) {
}
