#pragma once

#include "../../sub_system.h"
#include "client_command.h"
#include "operate_command.h"

#include <concepts>


class BASE_API UCommandSystem final : public ISubSystem {

    using AClientCreator = std::function<std::shared_ptr<IClientCommand>(const UCommandObject &)>;
    using AOperateCreator = std::function<std::shared_ptr<IOperateCommand>(const UCommandObject &)>;

    std::unordered_map<std::string, AClientCreator> clientMap_;
    std::unordered_map<std::string, AOperateCreator> operateMap_;

public:
    explicit UCommandSystem(UGameWorld *world);
    ~UCommandSystem() override;

    GET_SYSTEM_NAME(CommandSystem)

    void init() override;

    template<class T>
    requires std::derived_from<T, IClientCommand>
    void registerClientCommand(const std::string &type) {
        if (clientMap_.contains(type))
            return;

        clientMap_[type] = [this](const UCommandObject &obj) -> std::shared_ptr<IClientCommand> {
            return std::make_shared<T>(this, obj);
        };
    }

    template<class T>
    requires std::derived_from<T, IOperateCommand>
    void registerOperateCommand(const std::string &type) {
        if (operateMap_.contains(type))
            return;

        operateMap_[type] = [this](const UCommandObject &obj) -> std::shared_ptr<IOperateCommand> {
            return std::make_shared<T>(this, obj);
        };
    }

    [[nodiscard]] std::shared_ptr<IClientCommand> createClientCommand(const std::string &type, const std::string &args) const;
    [[nodiscard]] std::shared_ptr<IOperateCommand> createOperateCommand(const std::string &type, const std::string &args) const;
};
