#pragma once

#include "../Module.h"
#include "../utils.h"

#include <absl/container/flat_hash_map.h>

class UClazz;

class BASE_API UClazzFactory final : public IModule {

    DECLARE_MODULE(UClazzFactory)

    typedef void(*AReflectRegister)(UClazzFactory *);

    struct FLibraryNode {
        AModuleHandle mModule;
        AReflectRegister mFunctor;
    };

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
    std::vector<FLibraryNode> mNodeList;
    absl::flat_hash_map<std::string, UClazz *> mClazzMap;
};

