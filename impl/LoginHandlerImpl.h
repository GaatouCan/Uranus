#pragma once

#include "common.h"
#include <Login/LoginHandler.h>

class IMPL_API ULoginHandler final : public ILoginHandler {

public:
    explicit ULoginHandler(ULoginAuth *module);
    ~ULoginHandler() override;

    void UpdateAddressList() override;
    FLoginToken ParseLoginRequest(const std::shared_ptr<IPackageBase> &pkg) override;

    void OnLoginSuccess(int64_t pid, const std::shared_ptr<IPackageBase> &pkg) const override;
    void OnRepeatLogin(int64_t pid, const std::string &addr, const std::shared_ptr<IPackageBase> &pkg) override;
};
