#include "../include/package_pool.h"
#include "../include/package.h"

#include <spdlog/spdlog.h>


size_t UPackagePool::kDefaultCapacity = 64;
size_t UPackagePool::kMinCapacity = 16;

float UPackagePool::kExpanseRate = 0.3f;
float UPackagePool::kExpanseScale = 1.f;

float UPackagePool::kCollectRate = 1.f;
float UPackagePool::kCollectScale = 0.7f;

PackageCreator UPackagePool::kCreatePackage = nullptr;
PackageInitializer UPackagePool::kInitPackage = nullptr;


UPackagePool::UPackagePool(const size_t capacity) {
    for (size_t i = 0; i < capacity; i++) {
        IPackage *pkg = nullptr;
        pkg = kCreatePackage();

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
}

UPackagePool::~UPackagePool() {
    for (const auto it : pkg_set_) {
        delete it;
    }
    while (!queue_.empty()) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }
}

size_t UPackagePool::Capacity() const {
    std::shared_lock lock(mtx_);
    return queue_.size() + pkg_set_.size();
}

IPackage *UPackagePool::Acquire() {
    Expanse();

    IPackage *pkg = nullptr;

    {
        std::unique_lock lock(mtx_);
        pkg = queue_.front();
        queue_.pop();
        pkg_set_.insert(pkg);
        spdlog::trace("{} - Rest[{}], Current Use[{}]", __FUNCTION__, queue_.size(), pkg_set_.size());
    }

    if (InitPackage(pkg))
        return pkg;

    return nullptr;
}

void UPackagePool::Recycle(IPackage *pkg) {
    if (pkg == nullptr)
        return;

    {
        std::shared_lock lock(mtx_);
        if (!pkg_set_.contains(pkg))
            return;
    }

    pkg->Reset();

    {
        std::unique_lock lock(mtx_);

        queue_.push(pkg);
        pkg_set_.erase(pkg);
        spdlog::trace("{} - Rest[{}], Current Use[{}]", __FUNCTION__, queue_.size(), pkg_set_.size());
    }

    Collect();
}

void UPackagePool::LoadConfig(const YAML::Node &cfg) {
    if (cfg["package"].IsNull() && cfg["package"]["pool"].IsNull())
        return;

    if (!cfg["package"]["pool"]["default_capacity"].IsNull())
        SetDefaultCapacity(cfg["package"]["pool"]["default_capacity"].as<size_t>());

    if (!cfg["package"]["pool"]["minimum_capacity"].IsNull())
        SetMinimumCapacity(cfg["package"]["pool"]["minimum_capacity"].as<size_t>());

    if (!cfg["package"]["pool"]["expanse_rate"].IsNull())
        SetExpanseRate(cfg["package"]["pool"]["expanse_rate"].as<float>());

    if (!cfg["package"]["pool"]["expanse_scale"].IsNull())
        SetExpanseScale(cfg["package"]["pool"]["expanse_scale"].as<float>());

    if (!cfg["package"]["pool"]["collect_rate"].IsNull())
        SetCollectRate(cfg["package"]["pool"]["collect_rate"].as<float>());

    if (!cfg["package"]["pool"]["collect_scale"].IsNull())
        SetCollectScale(cfg["package"]["pool"]["collect_scale"].as<float>());

    spdlog::info("Package Pool Configuration Loaded Successfully.");
}

void UPackagePool::SetDefaultCapacity(const size_t capacity) {
    kDefaultCapacity = capacity;
}

void UPackagePool::SetMinimumCapacity(const size_t capacity) {
    kMinCapacity = capacity;
}

void UPackagePool::SetExpanseRate(const float rate) {
    kExpanseRate = rate;
}

void UPackagePool::SetExpanseScale(const float scale) {
    kExpanseScale = scale;
}

void UPackagePool::SetCollectRate(const float rate) {
    kCollectRate = rate;
}

void UPackagePool::SetCollectScale(const float scale) {
    kCollectScale = scale;
}

void UPackagePool::SetPackageBuilder(const PackageCreator& func) {
    kCreatePackage = func;
}

void UPackagePool::SetPackageInitializer(const PackageInitializer& func) {
    kInitPackage = func;
}

bool UPackagePool::InitPackage(IPackage *pkg) {
    if (pkg == nullptr)
        return false;

    kInitPackage(pkg);
    return true;
}

bool UPackagePool::HasAssignedBuilder() {
    return kCreatePackage != nullptr;
}

void UPackagePool::Expanse() {
    if (pkg_set_.empty() && !queue_.empty())
        return;

    if (std::floor(queue_.size() / Capacity()) <= kExpanseRate)
        return;

    const auto num = static_cast<size_t>(std::ceil(static_cast<float>(Capacity()) * kExpanseScale));
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}], Expand Number[{}].", __FUNCTION__, queue_.size(), pkg_set_.size(), num);

    std::unique_lock lock(mtx_);
    for (size_t i = 0; i < num; i++) {
        IPackage *pkg = nullptr;
        pkg = kCreatePackage();

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), pkg_set_.size());
}

void UPackagePool::Collect() {
    const auto now = NowTimePoint();

    // 不要太频繁
    if (now - collect_time_.load() < std::chrono::seconds(3))
        return;

    {
        std::unique_lock lock(mtx_);
        for (auto it = pkg_set_.begin(); it != pkg_set_.end();) {
            if (!(*it)->IsAvailable()) {
                (*it)->Reset();
                queue_.push(*it);
                it = pkg_set_.erase(it);
            } else
                ++it;
        }
    }

    if (queue_.size() <= kMinCapacity || std::floor(queue_.size() / Capacity()) < kCollectRate)
        return;

    collect_time_ = now;

    const auto num = static_cast<size_t>(std::floor(static_cast<float>(Capacity()) * kCollectScale));
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}], collect Number[{}].", __FUNCTION__, queue_.size(), pkg_set_.size(), num);

    std::unique_lock lock(mtx_);
    for (size_t i = 0; i < num && queue_.size() > kMinCapacity; i++) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }

    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), pkg_set_.size());
}
