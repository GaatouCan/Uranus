#pragma once

#include "utils.h"

#include <queue>
#include <set>
#include <shared_mutex>
#include <yaml-cpp/yaml.h>


class IPackage;

typedef IPackage*(*PackageCreator)();
typedef void(*PackageInitializer)(IPackage*);


/**
 * 数据包池
 */
class UPackagePool final {

    friend class UMainScene;

    explicit UPackagePool(size_t capacity = kDefaultCapacity);
    ~UPackagePool();

public:
    DISABLE_COPY_MOVE(UPackagePool)

    [[nodiscard]] size_t capacity() const;

    /**
     * 从池里获取一个数据包
     * @return 数据包基类指针
     */
    IPackage *acquire();

    /**
     * 回收数据包
     * @param pkg 被回收数据包指针
     */
    void recycle(IPackage *pkg);

    /**
     * 加载全局配置 定义扩容和收缩临界点和比例
     * @param cfg 全局配置
     */
    static void LoadConfig(const YAML::Node &cfg);

    static void SetDefaultCapacity(size_t capacity);
    static void SetMinimumCapacity(size_t capacity);

    static void SetExpanseRate(float rate);
    static void SetExpanseScale(float scale);

    static void SetCollectRate(float rate);
    static void SetCollectScale(float scale);

    static void SetPackageBuilder(const PackageCreator& func);
    static void SetPackageInitializer(const PackageInitializer& func);

    static bool InitPackage(IPackage *pkg);

    static bool HasAssignedBuilder();

private:
    void expanse();
    void collect();

private:
    std::queue<IPackage *> queue_;
    std::set<IPackage *> packageSet_;
    std::atomic<ATimePoint> collectTime_;

    mutable std::shared_mutex mutex_;

    // 扩容和收缩临界点和比例
    // 每个线程下的数据包池行为目前设计为一致

    static size_t kDefaultCapacity;
    static size_t kMinCapacity;

    static float kExpanseRate;
    static float kExpanseScale;

    static float kCollectRate;
    static float kCollectScale;

    static PackageCreator kCreatePackage;
    static PackageInitializer kInitPackage;
};
