#pragma once

#include "utils.h"

#include <queue>
#include <set>
#include <shared_mutex>
#include <yaml-cpp/yaml.h>


class IPackage;

typedef IPackage*(*PackageCreator)();
typedef void(*PackageInitializer)(IPackage*);

// using APackageCreator = std::function<IPackage*()>;
// using APackageInitializer = std::function<void(IPackage*)>;

/**
 * 数据包池
 */
class PackagePool final {

    friend class MainScene;
    // friend class UCrossRoute;

    explicit PackagePool(size_t capacity = default_capacity_);
    ~PackagePool();

public:
    DISABLE_COPY_MOVE(PackagePool)

    [[nodiscard]] size_t Capacity() const;

    /**
     * 从池里获取一个数据包
     * @return 数据包基类指针
     */
    IPackage *Acquire();

    /**
     * 回收数据包
     * @param pkg 被回收数据包指针
     */
    void Recycle(IPackage *pkg);

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

private:
    void Expanse();
    void Collect();

private:
    std::queue<IPackage *> queue_;
    std::set<IPackage *> set_;
    std::atomic<TimePoint> collect_time_;

    // std::mutex mMutex;
    mutable std::shared_mutex mutex_;

    // 扩容和收缩临界点和比例
    // 每个线程下的数据包池行为目前设计为一致

    static size_t default_capacity_;
    static size_t min_capacity_;

    static float expanse_rate_;
    static float expanse_scale_;

    static float collect_rate_;
    static float collect_scale_;

    static PackageCreator create_package_;
    static PackageInitializer init_package_;
};
