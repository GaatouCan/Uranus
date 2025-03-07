//
// Created by admin on 25-2-20.
//

#include "../../../include/system/command/command_system.h"

UCommandSystem::UCommandSystem(UGameWorld *world)
    : ISubSystem(world) {
}

UCommandSystem::~UCommandSystem() {

}

void UCommandSystem::Init() {

}

std::shared_ptr<IClientCommand> UCommandSystem::CreateClientCMD(const std::string &type, const std::string &args) const {
    if (const auto iter = client_map_.find(type); iter != client_map_.end())
        return iter->second(UCommandObject(type, args));
    return nullptr;
}

std::shared_ptr<IOperateCommand> UCommandSystem::CreateOperateCMD(const std::string &type, const std::string &args) const {
    if (const auto iter = operate_map_.find(type); iter != operate_map_.end())
        return iter->second(UCommandObject(type, args));
    return nullptr;
}


