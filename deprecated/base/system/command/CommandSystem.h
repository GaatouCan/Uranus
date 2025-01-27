#pragma once

#include "../../SubSystem.h"
#include "ClientCommand.h"
#include "OperateCommand.h"


class UCommandSystem final : public ISubSystem {

    using ACommandCreator = std::function<std::shared_ptr<IBaseCommand>(const UCommandObject&)>;

    std::unordered_map<std::string, ACommandCreator> mOperateCommandMap;
    std::unordered_map<std::string, ACommandCreator> mClientCommandMap;

public:
    SUB_SYSTEM_BODY(UCommandSystem)

    void Init() override;

    template<class T>
    requires std::derived_from<T, IClientCommand>
    void RegisterClientCommand(const std::string &cmd) {
        if (!mClientCommandMap.contains(cmd)) {
            mClientCommandMap[cmd] = [](const UCommandObject &obj) -> IBaseCommand* {
                return std::make_shared<T>(obj);
            };
        }
    }

    template<class T>
    requires std::derived_from<T, IOperateCommand>
    void RegisterOperateCommand(const std::string &cmd) {
        if (!mOperateCommandMap.contains(cmd)) {
            mOperateCommandMap[cmd] = [](const UCommandObject &obj) -> IBaseCommand* {
                return new T(obj);
            };
        }
    }

    [[nodiscard]] std::shared_ptr<IClientCommand> CreateClientCommand(const std::string &cmd, const std::string &args) const;
    [[nodiscard]] std::shared_ptr<IOperateCommand> CreateOperateCommand(const std::string &cmd, const std::string &args) const;
};
