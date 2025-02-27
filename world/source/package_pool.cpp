#include "../include/package_pool.h"
#include "../include/package.h"

#include <spdlog/spdlog.h>


size_t PackagePool::kDefaultCapacity = 64;
size_t PackagePool::kMinCapacity = 16;

float PackagePool::kExpanseRate = 0.3f;
float PackagePool::kExpanseScale = 1.f;

float PackagePool::kCollectRate = 1.f;
float PackagePool::kCollectScale = 0.7f;

PackageCreator PackagePool::kCreatePackage = nullptr;
PackageInitializer PackagePool::kInitPackage = nullptr;


PackagePool::PackagePool(const size_t capacity) {
    for (size_t i = 0; i < capacity; i++) {
        IPackage *pkg = nullptr;
        pkg = kCreatePackage();

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
}

PackagePool::~PackagePool() {
    for (const auto it : pkg_set_) {
        delete it;
    }
    while (!queue_.empty()) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }
}

size_t PackagePool::Capacity() const {
    std::shared_lock lock(mMutex);
    return queue_.size() + pkg_set_.size();
}

IPackage *PackagePool::Acquire() {
    Expanse();

    IPackage *pkg = nullptr;

    {
        std::unique_lock lock(mMutex);
        pkg = queue_.front();
        queue_.pop();
        pkg_set_.insert(pkg);
        spdlog::trace("{} - Rest[{}], Current Use[{}]", __FUNCTION__, queue_.size(), pkg_set_.size());
    }

    if (InitPackage(pkg))
        return pkg;

    return nullptr;
}

void PackagePool::Recycle(IPackage *pkg) {
    if (pkg == nullptr)
        return;

    {
        std::shared_lock lock(mMutex);
        if (!pkg_set_.contains(pkg))
            return;
    }

    pkg->Reset();

    {
        std::unique_lock lock(mMutex);

        queue_.push(pkg);
        pkg_set_.erase(pkg);
        spdlog::trace("{} - Rest[{}], Current Use[{}]", __FUNCTION__, queue_.size(), pkg_set_.size());
    }

    Collect();
}

void PackagePool::LoadConfig(const YAML::Node &cfg) {
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

void PackagePool::SetDefaultCapacity(const size_t capacity) {
    kDefaultCapacity = capacity;
}

void PackagePool::SetMinimumCapacity(const size_t capacity) {
    kMinCapacity = capacity;
}

void PackagePool::SetExpanseRate(const float rate) {
    kExpanseRate = rate;
}

void PackagePool::SetExpanseScale(const float scale) {
    kExpanseScale = scale;
}

void PackagePool::SetCollectRate(const float rate) {
    kCollectRate = rate;
}

void PackagePool::SetCollectScale(const float scale) {
    kCollectScale = scale;
}

void PackagePool::SetPackageBuilder(const PackageCreator& func) {
    kCreatePackage = func;
}

void PackagePool::SetPackageInitializer(const PackageInitializer& func) {
    kInitPackage = func;
}

bool PackagePool::InitPackage(IPackage *pkg) {
    if (pkg == nullptr)
        return false;

    kInitPackage(pkg);
    return true;
}

bool PackagePool::HasAssignedBuilder() {
    return kCreatePackage != nullptr;
}

void PackagePool::Expanse() {
    if (pkg_set_.empty() && !queue_.empty())
        return;

    if (std::floor(queue_.size() / Capacity()) <= kExpanseRate)
        return;

    const auto num = static_cast<size_t>(std::ceil(static_cast<float>(Capacity()) * kExpanseScale));
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}], Expand Number[{}].", __FUNCTION__, queue_.size(), pkg_set_.size(), num);

    std::unique_lock lock(mMutex);
    for (size_t i = 0; i < num; i++) {
        IPackage *pkg = nullptr;
        pkg = kCreatePackage();

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), pkg_set_.size());
}

void PackagePool::Collect() {
    const auto now = NowTimePoint();

    // 不要太频繁
    if (now - collect_time_.load() < std::chrono::seconds(3))
        return;

    {
        std::unique_lock lock(mMutex);
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

    std::unique_lock lock(mMutex);
    for (size_t i = 0; i < num && queue_.size() > kMinCapacity; i++) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }

    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), pkg_set_.size());
}
