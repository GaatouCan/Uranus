#pragma once

#include "common.h"

#include <concepts>

#ifdef __linux__
#include <cstdint>
#endif

/**
 * 抽象数据包基类
 */
class BASE_API IPackage {

public:
    virtual ~IPackage() = default;

    /**
     * 获取数据包ID
     * @return int
     */
    [[nodiscard]] virtual uint32_t getPackageID() const { return -1; }

    /**
     * 清空数据包内所有数据
     */
    virtual void reset() = 0;

    /**
     * 使数据包变为失效状态
     */
    virtual void invalid() = 0;

    /**
     * 判断数据包是否可用
     * @return bool
     */
    [[nodiscard]] virtual bool available() const = 0;

    virtual void copyFrom(IPackage *other) = 0;
};

template<typename T>
concept CPackageType = std::derived_from<T, IPackage>;
