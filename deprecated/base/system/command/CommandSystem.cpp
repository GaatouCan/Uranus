#include "CommandSystem.h"
#include "../../GameWorld.h"

void UCommandSystem::Init() {
}

SUB_SYSTEM_IMPL(UCommandSystem)

std::shared_ptr<IClientCommand> UCommandSystem::CreateClientCommand(const std::string &cmd, const std::string &args) const {
    if (const auto it = mClientCommandMap.find(cmd); it != mClientCommandMap.end()) {
        UCommandObject obj(cmd, args);
        const auto cmdPtr = std::invoke(it->second, obj);
        return std::dynamic_pointer_cast<IClientCommand>(cmdPtr);
    }
    return nullptr;
}

std::shared_ptr<IOperateCommand> UCommandSystem::CreateOperateCommand(const std::string &cmd, const std::string &args) const {
    if (const auto it = mOperateCommandMap.find(cmd); it != mOperateCommandMap.end()) {
        UCommandObject obj(cmd, args);
        const auto cmdPtr = std::invoke(it->second, obj);
        return std::dynamic_pointer_cast<IOperateCommand>(cmdPtr);
    }
    return nullptr;
}
