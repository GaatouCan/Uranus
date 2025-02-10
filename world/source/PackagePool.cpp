#include "../include/PackagePool.h"
#include "../include/Package.h"

#include <spdlog/spdlog.h>


size_t UPackagePool::defaultCapacity = 64;
size_t UPackagePool::minCapacity = 16;

float UPackagePool::expanseRate = 0.3f;
float UPackagePool::expanseScale = 1.f;

float UPackagePool::collectRate = 1.f;
float UPackagePool::collectScale = 0.7f;

APackageCreator UPackagePool::createPackage = nullptr;
APackageInitializer UPackagePool::initPackage = nullptr;


UPackagePool::UPackagePool(const size_t capacity) {
    for (size_t i = 0; i < capacity; i++) {
        IPackage *pkg = nullptr;
        pkg = std::invoke(createPackage);

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
}

UPackagePool::~UPackagePool() {
    for (const auto it : set_) {
        delete it;
    }
    while (!queue_.empty()) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }
}

size_t UPackagePool::Capacity() const {
    std::shared_lock lock(mutex_);
    return queue_.size() + set_.size();
}

IPackage *UPackagePool::Acquire() {
    Expanse();

    IPackage *pkg = nullptr;

    {
        std::unique_lock lock(mutex_);
        pkg = queue_.front();
        queue_.pop();
        set_.insert(pkg);
        spdlog::trace("{} - Rest[{}], Current Use[{}]", __FUNCTION__, queue_.size(), set_.size());
    }

    if (InitPackage(pkg))
        return pkg;

    return nullptr;
}

void UPackagePool::Recycle(IPackage *pkg) {
    if (pkg == nullptr)
        return;

    pkg->Reset();

    {
        std::unique_lock lock(mutex_);

        queue_.push(pkg);
        set_.erase(pkg);
        spdlog::trace("{} - Rest[{}], Current Use[{}]", __FUNCTION__, queue_.size(), set_.size());
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
    defaultCapacity = capacity;
}

void UPackagePool::SetMinimumCapacity(const size_t capacity) {
    minCapacity = capacity;
}

void UPackagePool::SetExpanseRate(const float rate) {
    expanseRate = rate;
}

void UPackagePool::SetExpanseScale(const float scale) {
    expanseScale = scale;
}

void UPackagePool::SetCollectRate(const float rate) {
    collectRate = rate;
}

void UPackagePool::SetCollectScale(const float scale) {
    collectScale = scale;
}

void UPackagePool::SetPackageBuilder(const APackageCreator& func) {
    createPackage = func;
}

void UPackagePool::SetPackageInitializer(const APackageInitializer& func) {
    initPackage = func;
}

bool UPackagePool::InitPackage(IPackage *pkg) {
    if (pkg == nullptr)
        return false;

    std::invoke(initPackage, pkg);
    return true;
}

void UPackagePool::Expanse() {
    if (set_.empty() && !queue_.empty())
        return;

    if (std::floor(queue_.size() / Capacity()) <= expanseRate)
        return;

    const auto num = static_cast<size_t>(std::ceil(static_cast<float>(Capacity()) * expanseScale));
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}], Expand Number[{}].", __FUNCTION__, queue_.size(), set_.size(), num);

    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < num; i++) {
        IPackage *pkg = nullptr;
        pkg = std::invoke(createPackage);

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), set_.size());
}

void UPackagePool::Collect() {
    const auto now = NowTimePoint();

    // 不要太频繁
    if (now - collectTime_.load() < std::chrono::seconds(3))
        return;

    {
        std::unique_lock lock(mutex_);
        for (auto it = set_.begin(); it != set_.end();) {
            if (!(*it)->IsAvailable()) {
                (*it)->Reset();
                queue_.push(*it);
                it = set_.erase(it);
            } else
                ++it;
        }
    }

    if (queue_.size() <= minCapacity || std::floor(queue_.size() / Capacity()) < collectRate)
        return;

    collectTime_ = now;

    const auto num = static_cast<size_t>(std::floor(static_cast<float>(Capacity()) * collectScale));
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}], collect Number[{}].", __FUNCTION__, queue_.size(), set_.size(), num);

    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < num && queue_.size() > minCapacity; i++) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }

    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), set_.size());
}
