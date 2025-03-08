//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/command_system.h"

UCommandSystem::UCommandSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UCommandSystem::~UCommandSystem() {

}

void UCommandSystem::init() {

}

std::shared_ptr<IClientCommand> UCommandSystem::createClientCommand(const std::string &type, const std::string &args) const {
    if (const auto iter = clientMap_.find(type); iter != clientMap_.end())
        return iter->second(UCommandObject(type, args));
    return nullptr;
}

std::shared_ptr<IOperateCommand> UCommandSystem::createOperateCommand(const std::string &type, const std::string &args) const {
    if (const auto iter = operateMap_.find(type); iter != operateMap_.end())
        return iter->second(UCommandObject(type, args));
    return nullptr;
}


