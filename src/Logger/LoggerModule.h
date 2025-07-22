#pragma once

#include "Module.h"

#include <absl/container/flat_hash_map.h>
#include <shared_mutex>


class BASE_API ULoggerModule final : public IModuleBase {

    DECLARE_MODULE(ULoggerModule)

protected:
    ULoggerModule();
    ~ULoggerModule() override;

    void Stop() override;

public:
    [[nodiscard]] constexpr const char * GetModuleName() const override {
        return "Logger Module";
    }

    void TryCreateLogger(const std::string &name);
    void TryDestroyLogger(const std::string &name);

    [[nodiscard]] int GetLoggerUseCount(const std::string &name) const;

private:
    absl::flat_hash_map<std::string, int> mUseCount;
    mutable std::shared_mutex mMutex;
};

