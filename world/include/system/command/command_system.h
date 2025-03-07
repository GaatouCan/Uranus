#pragma once

#include "../../sub_system.h"
#include "client_command.h"
#include "operate_command.h"

#include <concepts>


class BASE_API UCommandSystem final : public ISubSystem {

    using AClientCreator = std::function<std::shared_ptr<IClientCommand>(const UCommandObject &)>;
    using AOperateCreator = std::function<std::shared_ptr<IOperateCommand>(const UCommandObject &)>;

    std::unordered_map<std::string, AClientCreator> client_map_;
    std::unordered_map<std::string, AOperateCreator> operate_map_;

public:
    explicit UCommandSystem(UGameWorld *world);
    ~UCommandSystem() override;

    GET_SYSTEM_NAME(CommandSystem)

    void Init() override;

    template<class T>
    requires std::derived_from<T, IClientCommand>
    void RegisterClientCommand(const std::string &type) {
        if (client_map_.contains(type))
            return;

        client_map_[type] = [this](const UCommandObject &obj) -> std::shared_ptr<IClientCommand> {
            return std::make_shared<T>(this, obj);
        };
    }

    template<class T>
    requires std::derived_from<T, IOperateCommand>
    void RegisterOperateCommand(const std::string &type) {
        if (operate_map_.contains(type))
            return;

        operate_map_[type] = [this](const UCommandObject &obj) -> std::shared_ptr<IOperateCommand> {
            return std::make_shared<T>(this, obj);
        };
    }

    [[nodiscard]] std::shared_ptr<IClientCommand> CreateClientCMD(const std::string &type, const std::string &args) const;
    [[nodiscard]] std::shared_ptr<IOperateCommand> CreateOperateCMD(const std::string &type, const std::string &args) const;
};
