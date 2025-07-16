#pragma once


#include "../Module.h"
#include "../Utils.h"

#include <shared_mutex>
#include <absl/container/flat_hash_map.h>

class UClazz;

class BASE_API UClazzFactory final : public IModule {

    DECLARE_MODULE(UClazzFactory)


protected:
    explicit UClazzFactory(UServer *server);

    void Initial() override;

public:
    ~UClazzFactory() override;
    [[nodiscard]] constexpr const char *GetModuleName() const override {
        return "ClazzFactory";
    }

    void RegisterClazz(UClazz *clazz);
    [[nodiscard]] UClazz *FromName(const std::string &name) const;

    void RemoveClazz(const std::string &name);

private:
    mutable std::shared_mutex mMutex;
    absl::flat_hash_map<std::string, UClazz *> mClazzMap;
};

