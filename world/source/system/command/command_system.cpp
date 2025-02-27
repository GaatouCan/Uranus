//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/command_system.h"

CommandSystem::CommandSystem(GameWorld *world)
    : ISubSystem(world) {
}

CommandSystem::~CommandSystem() {

}

void CommandSystem::Init() {

}

std::shared_ptr<IClientCommand> CommandSystem::CreateClientCMD(const std::string &type, const std::string &args) const {
    if (const auto iter = client_map_.find(type); iter != client_map_.end())
        return iter->second(CommandObject(type, args));
    return nullptr;
}

std::shared_ptr<IOperateCommand> CommandSystem::CreateOperateCMD(const std::string &type, const std::string &args) const {
    if (const auto iter = operate_map_.find(type); iter != operate_map_.end())
        return iter->second(CommandObject(type, args));
    return nullptr;
}


