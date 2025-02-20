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
    if (const auto iter = mClientMap.find(type); iter != mClientMap.end())
        return iter->second(CommandObject(type, args));
    return nullptr;
}

std::shared_ptr<IOperateCommand> CommandSystem::CreateOperateCMD(const std::string &type, const std::string &args) const {
    if (const auto iter = mOperateMap.find(type); iter != mOperateMap.end())
        return iter->second(CommandObject(type, args));
    return nullptr;
}


