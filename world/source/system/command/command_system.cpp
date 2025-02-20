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
    if (const auto iter = mClientMap.find(type); iter != mClientMap.end()) {
        const CommandObject obj(type, args);
        const auto res = iter->second(obj);
        return std::dynamic_pointer_cast<IClientCommand>(res);
    }
    return nullptr;
}

std::shared_ptr<IOperateCommand> CommandSystem::CreateOperateCMD(const std::string &type, const std::string &args) const {
    if (const auto iter = mOperateMap.find(type); iter != mOperateMap.end()) {
        const CommandObject obj(type, args);
        const auto res = iter->second(obj);
        return std::dynamic_pointer_cast<IOperateCommand>(res);
    }
    return nullptr;
}


