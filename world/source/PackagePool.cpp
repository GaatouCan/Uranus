#include "../include/PackagePool.h"
#include "../include/Package.h"

#include <spdlog/spdlog.h>


size_t PackagePool::default_capacity_ = 64;
size_t PackagePool::min_capacity_ = 16;

float PackagePool::expanse_rate_ = 0.3f;
float PackagePool::expanse_scale_ = 1.f;

float PackagePool::collect_rate_ = 1.f;
float PackagePool::collect_scale_ = 0.7f;

PackageCreator PackagePool::create_package_ = nullptr;
PackageInitializer PackagePool::init_package_ = nullptr;


PackagePool::PackagePool(const size_t capacity) {
    for (size_t i = 0; i < capacity; i++) {
        IPackage *pkg = nullptr;
        pkg = create_package_();

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
}

PackagePool::~PackagePool() {
    for (const auto it : set_) {
        delete it;
    }
    while (!queue_.empty()) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }
}

size_t PackagePool::Capacity() const {
    std::shared_lock lock(mutex_);
    return queue_.size() + set_.size();
}

IPackage *PackagePool::Acquire() {
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

void PackagePool::Recycle(IPackage *pkg) {
    if (pkg == nullptr)
        return;

    {
        std::shared_lock lock(mutex_);
        if (!set_.contains(pkg))
            return;
    }

    pkg->Reset();

    {
        std::unique_lock lock(mutex_);

        queue_.push(pkg);
        set_.erase(pkg);
        spdlog::trace("{} - Rest[{}], Current Use[{}]", __FUNCTION__, queue_.size(), set_.size());
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
    default_capacity_ = capacity;
}

void PackagePool::SetMinimumCapacity(const size_t capacity) {
    min_capacity_ = capacity;
}

void PackagePool::SetExpanseRate(const float rate) {
    expanse_rate_ = rate;
}

void PackagePool::SetExpanseScale(const float scale) {
    expanse_scale_ = scale;
}

void PackagePool::SetCollectRate(const float rate) {
    collect_rate_ = rate;
}

void PackagePool::SetCollectScale(const float scale) {
    collect_scale_ = scale;
}

void PackagePool::SetPackageBuilder(const PackageCreator& func) {
    create_package_ = func;
}

void PackagePool::SetPackageInitializer(const PackageInitializer& func) {
    init_package_ = func;
}

bool PackagePool::InitPackage(IPackage *pkg) {
    if (pkg == nullptr)
        return false;

    init_package_(pkg);
    return true;
}

void PackagePool::Expanse() {
    if (set_.empty() && !queue_.empty())
        return;

    if (std::floor(queue_.size() / Capacity()) <= expanse_rate_)
        return;

    const auto num = static_cast<size_t>(std::ceil(static_cast<float>(Capacity()) * expanse_scale_));
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}], Expand Number[{}].", __FUNCTION__, queue_.size(), set_.size(), num);

    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < num; i++) {
        IPackage *pkg = nullptr;
        pkg = create_package_();

        if (pkg != nullptr) {
            queue_.push(pkg);
        }
    }
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), set_.size());
}

void PackagePool::Collect() {
    const auto now = NowTimePoint();

    // 不要太频繁
    if (now - collect_time_.load() < std::chrono::seconds(3))
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

    if (queue_.size() <= min_capacity_ || std::floor(queue_.size() / Capacity()) < collect_rate_)
        return;

    collect_time_ = now;

    const auto num = static_cast<size_t>(std::floor(static_cast<float>(Capacity()) * collect_scale_));
    spdlog::trace("{} - Pool Rest[{}], Current Using[{}], collect Number[{}].", __FUNCTION__, queue_.size(), set_.size(), num);

    std::unique_lock lock(mutex_);
    for (size_t i = 0; i < num && queue_.size() > min_capacity_; i++) {
        const auto pkg = queue_.front();
        queue_.pop();
        delete pkg;
    }

    spdlog::trace("{} - Pool Rest[{}], Current Using[{}].", __FUNCTION__, queue_.size(), set_.size());
}
