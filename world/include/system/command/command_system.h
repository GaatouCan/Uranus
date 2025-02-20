#pragma once

#include "../../sub_system.h"
#include "client_command.h"
#include "operate_command.h"

#include <concepts>


class BASE_API CommandSystem final : public ISubSystem {

    using ClientCreator = std::function<std::shared_ptr<IClientCommand>(const CommandObject &)>;
    using OperateCreator = std::function<std::shared_ptr<IOperateCommand>(const CommandObject &)>;

    std::unordered_map<std::string, ClientCreator> mClientMap;
    std::unordered_map<std::string, OperateCreator> mOperateMap;

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

        mClientMap[type] = [](const CommandObject &obj) -> std::shared_ptr<IClientCommand> {
            return std::make_shared<T>(obj);
        };
    }

    template<class T>
    requires std::derived_from<T, IOperateCommand>
    void RegisterOperateCommand(const std::string &type) {
        if (mOperateMap.contains(type))
            return;

        mOperateMap[type] = [](const CommandObject &obj) -> std::shared_ptr<IOperateCommand> {
            return std::make_shared<T>(obj);
        };
    }

    [[nodiscard]] std::shared_ptr<IClientCommand> CreateClientCMD(const std::string &type, const std::string &args) const;
    [[nodiscard]] std::shared_ptr<IOperateCommand> CreateOperateCMD(const std::string &type, const std::string &args) const;
};
