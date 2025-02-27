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
    [[nodiscard]] virtual uint32_t GetPackageID() const { return -1; }

    /**
     * 清空数据包内所有数据
     */
    virtual void Reset() = 0;

    /**
     * 使数据包变为失效状态
     */
    virtual void Invalid() = 0;

    /**
     * 判断数据包是否可用
     * @return bool
     */
    [[nodiscard]] virtual bool IsAvailable() const = 0;

    virtual void CopyFrom(IPackage *other) = 0;
};

template<typename T>
concept PackageType = std::derived_from<T, IPackage>;
