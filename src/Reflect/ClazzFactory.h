#pragma once

#include "../Module.h"
#include "../utils.h"

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

private:
    absl::flat_hash_map<std::string, UClazz *> mClazzMap;
};

