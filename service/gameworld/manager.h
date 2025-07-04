#pragma once

#include <common.h>
#include <concepts>

class IManager {

    friend class UGameWorld;

protected:
    explicit IManager(UGameWorld *world);

    virtual void Initial();

    virtual void BeginPlay();
    virtual void EndPlay();

public:
    IManager() = delete;
    virtual ~IManager();

    DISABLE_COPY_MOVE(IManager)

    [[nodiscard]] virtual const char *GetManagerName() const = 0;
    [[nodiscard]] UGameWorld *GetWorld() const;

private:
    UGameWorld *world_;
};

template<class Type>
concept CManagerType = std::derived_from<Type, IManager> && !std::is_same_v<Type, IManager>;
