#pragma once

#include "../../sub_system.h"
#include "client_command.h"
#include "operate_command.h"

#include <concepts>


class BASE_API CommandSystem final : public ISubSystem {

    using Creator = std::function<std::shared_ptr<IBaseCommand>(const CommandObject &)>;

    std::unordered_map<std::string, Creator> mClientMap;
    std::unordered_map<std::string, Creator> mOperateMap;

public:
    explicit CommandSystem(GameWorld *world);
    ~CommandSystem() override;

    GET_SYSTEM_NAME(CommandSystem)

    void Init() override;

    template<class T>
    requires std::derived_from<T, IClientCommand>
    void RegisterClientCommand(const std::string &type) {
        if (mClientMap.contains(type))
            return;

        mClientMap[type] = [](const CommandObject &obj) -> std::shared_ptr<IBaseCommand> {
            return std::make_shared<T>(obj);
        };
    }

    template<class T>
    requires std::derived_from<T, IOperateCommand>
    void RegisterOperateCommand(const std::string &type) {
        if (mOperateMap.contains(type))
            return;

        mOperateMap[type] = [](const CommandObject &obj) -> std::shared_ptr<IBaseCommand> {
            return std::make_shared<T>(obj);
        };
    }

    std::shared_ptr<IClientCommand> CreateClientCMD(const std::string &type, const std::string &args) const;
    std::shared_ptr<IOperateCommand> CreateOperateCMD(const std::string &type, const std::string &args) const;
};
