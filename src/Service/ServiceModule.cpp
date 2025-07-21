#include "ServiceModule.h"
#include "Service.h"
#include "Package.h"
#include "LibraryHandle.h"
#include "Config/Config.h"

#include <spdlog/spdlog.h>


UServiceModule::UServiceModule() {
}

UServiceModule::~UServiceModule() {
    Stop();
}

void UServiceModule::Initial() {
    if (mState != EModuleState::CREATED)
        return;

    const auto *configModule = GetServer()->GetModule<UConfig>();
    if (configModule == nullptr) {
        SPDLOG_ERROR("Config Module Not Found, Service Module Initialization Failed!");
        GetServer()->Shutdown();
        exit(-6);
    }

    const auto &config = configModule->GetServerConfig();

    // Find Dynamic Library From Directories

#ifdef __linux__
    const std::string prefix = "lib";
#endif

    SPDLOG_INFO("Loading Core Service...");
    flat_hash_map<std::string, std::filesystem::path> coreMap;

    if (config["service"] && config["service"]["core"]) {
        for (const auto &val : config["service"]["core"]) {
            const auto filename = val["name"].as<std::string>();
#if defined(_WIN32) || defined(_WIN64)
            const std::filesystem::path path = std::filesystem::path(CORE_SERVICE_DIRECTORY) / (filename + ".dll");
#else
            std::string linux_filename = filename;
            if (filename.compare(0, prefix.size(), prefix) != 0) {
                linux_filename = prefix + filename;
            }
            const std::filesystem::path path = std::filesystem::path(CORE_SERVICE_DIRECTORY) / (linux_filename + ".so");
#endif
            coreMap.emplace(filename, path);
        }
    } else {
        for (const auto &entry: std::filesystem::directory_iterator(CORE_SERVICE_DIRECTORY)) {
#if defined(_WIN32) || defined(_WIN64)
            if (entry.is_regular_file() && entry.path().extension() == ".dll") {
                const auto filename = entry.path().stem().string();
#else
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                auto filename = entry.path().stem().string();
                if (filename.compare(0, prefix.size(), prefix) == 0) {
                    filename.erase(0, prefix.size());
                }
#endif
                coreMap.emplace(filename, entry.path());
            }
        }
    }

    for (const auto &[filename, path] : coreMap) {
        if (auto node = new FLibraryHandle(); node->LoadFrom(path.string())) {
            mCoreHandleMap[filename] = node;
            SPDLOG_INFO("\tLoaded Core Service Library From[{}]", path.string());
        }
    }

    SPDLOG_INFO("Loading Extend Service...");
    flat_hash_map<std::string, std::filesystem::path> extendMap;

    if (config["service"] && config["service"]["extend"]) {
        for (const auto &val : config["service"]["extend"]) {
            const auto filename = val["name"].as<std::string>();
#if defined(_WIN32) || defined(_WIN64)
            const std::filesystem::path path = std::filesystem::path(EXTEND_SERVICE_DIRECTORY) / (filename + ".dll");
#else
            std::string linux_filename = filename;
            if (filename.compare(0, prefix.size(), prefix) != 0) {
                linux_filename = prefix + filename;
            }
            const std::filesystem::path path = std::filesystem::path(EXTEND_SERVICE_DIRECTORY) / (linux_filename + ".so");
#endif
            extendMap[filename] = path;
        }
    } else {
        for (const auto &entry: std::filesystem::directory_iterator(EXTEND_SERVICE_DIRECTORY)) {
#if defined(_WIN32) || defined(_WIN64)
            if (entry.is_regular_file() && entry.path().extension() == ".dll") {
                const auto filename = entry.path().stem().string();
#else
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                auto filename = entry.path().stem().string();
                if (filename.compare(0, prefix.size(), prefix) == 0) {
                    filename.erase(0, prefix.size());
                }
#endif
                extendMap[filename] = entry.path();
            }
        }
    }

    for (const auto &[filename, path] : extendMap) {
        if (auto *node = new FLibraryHandle(); node->LoadFrom(path.string())) {
            mExtendHandleMap[filename] = node;
            SPDLOG_INFO("\tLoaded Extend Service Library From[{}]", path.string());
        }
    }

    // Begin Initial Core Service

    for (const auto &[filename, node] : mCoreHandleMap) {
        const int32_t sid = mAllocator.Allocate();
        const auto context = std::make_shared<UContext>();

        context->SetUpModule(this);
        context->SetUpHandle(node);
        context->SetServiceID(sid);

        if (context->Initial(nullptr)) {
            const auto name = context->GetServiceName();

            mServiceMap[sid] = context;
            mFilenameMapping[filename].insert(sid);
            mServiceInfoMap[name] = {sid, filename, name, true};

            SPDLOG_INFO("Initialized Service[{}]", name);
        } else {
            SPDLOG_ERROR("Failed To Initialize Service[{}]", filename);
            GetServer()->Shutdown();
            exit(-8);
        }
    }

    mState = EModuleState::INITIALIZED;
}

void UServiceModule::Start() {
    if (mState != EModuleState::INITIALIZED)
        return;

    for (const auto &context: mServiceMap | std::views::values) {
        if (context->BootService()) {
            SPDLOG_INFO("Starting Core Service[{}]...", context->GetServiceName());
        } else {
            SPDLOG_ERROR("Failed To Start Core Service[{}]", context->GetServiceName());
            GetServer()->Shutdown();
            exit(-9);
        }
    }

    mState = EModuleState::RUNNING;
}

void UServiceModule::Stop() {
    if (mState == EModuleState::STOPPED)
        return;

    mState = EModuleState::STOPPED;

    SPDLOG_INFO("Unloading All Service...");
    for (const auto &context : mServiceMap | std::views::values) {
        context->ForceShutdown();
    }

    for (const auto &node : mCoreHandleMap | std::views::values) {
        delete node;
    }

    for (const auto &node : mExtendHandleMap | std::views::values) {
        delete node;
    }

    SPDLOG_INFO("Free All Service Library Successfully");
}

std::shared_ptr<UContext> UServiceModule::BootExtendService(const std::string &filename, const std::shared_ptr<IPackageInterface> &pkg) {
    if (mState != EModuleState::RUNNING)
        return nullptr;

    const auto handle = FindServiceHandle(filename);
    if (handle == nullptr) {
        SPDLOG_WARN("{:<20} - Cannot Found Service Library[{}]", __FUNCTION__, filename);
        return nullptr;
    }

    const int32_t sid = mAllocator.AllocateT();
    if (sid < 0)
        return nullptr;

    auto context = std::make_shared<UContext>();

    context->SetUpModule(this);
    context->SetUpHandle(handle);
    context->SetServiceID(sid);

    if (!context->Initial(pkg)) {
        SPDLOG_ERROR("{:<20} - Failed To Initial Service[{}]", __FUNCTION__, filename);

        context->ForceShutdown();
        mAllocator.RecycleT(sid);

        return nullptr;
    }

    // Check If Service Name Unique
    bool bSuccess = true;

    {
        std::shared_lock lock(mInfoMutex);
        if (mServiceInfoMap.contains(context->GetServiceName())) {
            SPDLOG_WARN("{:<20} - Service[{}] Has Already Exist.", __FUNCTION__, context->GetServiceName());
            bSuccess = false;
        }
    }

    // If Service Name Unique
    if (bSuccess) {
        if (context->BootService()) {
            {
                const auto name = context->GetServiceName();

                std::scoped_lock lock(mServiceMutex, mFileNameMutex, mInfoMutex);

                mServiceMap[sid] = context;
                mFilenameMapping[filename].insert(sid);
                mServiceInfoMap[name] = {sid, filename, name, false};
            }

            SPDLOG_INFO("{:<20} - Boot Extend Service[{}] Successfully", __FUNCTION__, context->GetServiceName());
            return context;
        }

        SPDLOG_ERROR("{:<20} - Failed To Boot Extend Service[{}]", __FUNCTION__, context->GetServiceName());
    }

    // If Not Unique, Service Force To Shut Down And Recycle The Service ID
    context->ForceShutdown();
    mAllocator.RecycleT(sid);

    return nullptr;
}

void UServiceModule::ShutdownService(const int32_t id) {
    if (mState != EModuleState::RUNNING)
        return;

    std::shared_ptr<UContext> context;
    const auto info = GetContextInfo(id);

    if (info.id == INVALID_SERVICE_ID) {
        SPDLOG_ERROR("{:<20} - Can't Find Service[{}]", __FUNCTION__, id);
        return;
    }

    if (info.bCore) {
        // TODO
    } else {
        {
            std::scoped_lock lock(mServiceMutex, mInfoMutex);
            if (const auto &iter = mServiceMap.find(id); iter != mServiceMap.end()) {
                context = iter->second;
                mServiceMap.erase(iter);
            }

            mServiceInfoMap.erase(info.name);
        }

        if (context == nullptr) {
            mAllocator.RecycleT(id);
            return;
        }

        auto func = [this, id, filename = info.filename](IContextBase *) {
            if (mState != EModuleState::RUNNING)
                return;
            OnServiceShutdown(filename, id, false);
            mAllocator.RecycleT(id);
        };

        context->Shutdown(false, 5, func);
    }
}

std::shared_ptr<UContext> UServiceModule::FindService(const int32_t id) const {
    if (mState != EModuleState::RUNNING)
        return nullptr;

    std::shared_lock lock(mServiceMutex);
    const auto iter = mServiceMap.find(id);
    return iter != mServiceMap.end() ? iter->second : nullptr;
}

std::shared_ptr<UContext> UServiceModule::FindService(const std::string &name) const {
    if (mState != EModuleState::RUNNING)
        return nullptr;

    int32_t sid;

    {
        std::shared_lock lock(mInfoMutex);
        const auto nameIter = mServiceInfoMap.find(name);
        if (nameIter == mServiceInfoMap.end())
            return nullptr;

        sid = nameIter->second.id;
    }

    std::shared_lock lock(mServiceMutex);
    const auto iter = mServiceMap.find(sid);
    return iter != mServiceMap.end() ? iter->second : nullptr;
}

std::map<std::string, int32_t> UServiceModule::GetServiceList() const {
    if (mState != EModuleState::RUNNING)
        return {};

    std::shared_lock lock(mInfoMutex);
    std::map<std::string, int32_t> result;
    for (const auto &[fst, snd] : mServiceInfoMap) {
        result.emplace(fst, snd.id);
    }
    return result;
}

int32_t UServiceModule::GetServiceID(const std::string &name) const {
    if (mState != EModuleState::INITIALIZED || mState != EModuleState::RUNNING)
        return -10;

    std::shared_lock lock(mInfoMutex);
    const auto nameIter = mServiceInfoMap.find(name);
    return nameIter != mServiceInfoMap.end() ? nameIter->second.id : -10;
}

void UServiceModule::LoadLibraryFrom(const std::string &filename, const bool bCore) {
    if (mState != EModuleState::RUNNING)
        return;

#ifdef __linux__
    const std::string prefix = "lib";
#endif

    if (bCore) {
        // TODO:
    } else {
        std::unique_lock lock(mHandleMutex);
        if (mExtendHandleMap.contains(filename)) {
            SPDLOG_INFO("{:<20} - Library[{}] Has Already Exist.", __FUNCTION__, filename);
            return;
        }

#if defined(_WIN32) || defined(_WIN64)
        const std::filesystem::path path = std::filesystem::path(EXTEND_SERVICE_DIRECTORY) / (filename + ".dll");
#else
        std::string linux_filename = filename;
        if (filename.compare(0, prefix.size(), prefix) != 0) {
            linux_filename = prefix + filename;
        }
        const std::filesystem::path path = std::filesystem::path(EXTEND_SERVICE_DIRECTORY) / (linux_filename + ".so");
#endif

        if (const auto node = new FLibraryHandle(); node->LoadFrom(path.string())) {
            mExtendHandleMap[filename] = node;
            SPDLOG_INFO("{:<20} - Successfully Loaded Extend Library From[{}]", __FUNCTION__, filename);
        }
    }
}

void UServiceModule::UnloadLibrary(const std::string &filename, const bool bCore) {
    if (mState != EModuleState::RUNNING)
        return;

    if (bCore) {
        // TODO: Core Service Not Implement Now
    } else {
        // Check If It Is Extend Library Path
        {
            std::shared_lock lock(mHandleMutex);
            if (!mExtendHandleMap.contains(filename)) {
                SPDLOG_WARN("{:<20} - [{}] Is An Invalid Extend Service Library Filename", __FUNCTION__, filename);
                return;
            }
        }

        SPDLOG_INFO("{:<20} - Waiting Shut Down All Extend Services Created From Library[{}]...", __FUNCTION__, filename);

        flat_hash_set<int32_t> del;
        // bool bComplete = true;

        // Get All Services Need To Shut Down
        {
            std::unique_lock lock(mFileNameMutex);
            if (const auto iter = mFilenameMapping.find(filename); iter != mFilenameMapping.end()) {
                del = iter->second;
            }
        }

        for (const auto &sid : del) {
            std::shared_ptr<UContext> context;

            {
                std::scoped_lock lock(mServiceMutex, mInfoMutex);
                if (const auto iter = mServiceMap.find(sid); iter != mServiceMap.end()) {
                    context = iter->second;
                    mServiceMap.erase(iter);
                    mServiceInfoMap.erase(context->GetServiceName());
                }
            }

            auto func = [this, filename, sid](IContextBase *ptr) {
                if (mState != EModuleState::RUNNING)
                    return;

                const bool bCanDelete = OnServiceShutdown(filename, sid, false);
                mAllocator.RecycleT(sid);

                if (bCanDelete) {
                    std::unique_lock lock(mHandleMutex);
                    if (const auto iter = mExtendHandleMap.find(filename); iter != mExtendHandleMap.end()) {
                        delete iter->second;
                        mExtendHandleMap.erase(iter);
                    }
                    SPDLOG_INFO("Free Service Library[{}] Successfully", filename);
                }
            };

            if (context != nullptr) {
                if (context->Shutdown(false, 5, func) == 0) {
                    // bComplete = false;
                    SPDLOG_INFO("{:<20} - Extend Service[{} - {}] Waiting Complete", __FUNCTION__, sid, context->GetServiceName());
                } else {
                    SPDLOG_INFO("{:<20} - Extend Service[{}] Shutdown", __FUNCTION__, sid);
                }
            }
        }

        // if (bComplete) {
        //     {
        //         std::unique_lock lock(mFileNameMutex);
        //         if (const auto iter = mFilenameToServiceID.find(path); iter != mFilenameToServiceID.end()) {
        //             mFilenameToServiceID.erase(iter);
        //         }
        //     }
        //     SPDLOG_INFO("{:<20} - All Extend Services Created From[{}] Shutdown", __FUNCTION__, path);
        // }
    }
}

UServiceModule::FServiceInfo UServiceModule::GetContextInfo(int32_t id) const {
    if (GetState() != EModuleState::RUNNING)
        return {};

    const auto context = FindService(id);
    if (context == nullptr)
        return {};

    std::shared_lock lock(mInfoMutex);
    const auto iter = mServiceInfoMap.find(context->GetServiceName());
    return iter == mServiceInfoMap.end() ? FServiceInfo{} : iter->second;
}

FLibraryHandle *UServiceModule::FindServiceHandle(const std::string &path, const bool bCore) const {
    std::shared_lock lock(mHandleMutex);
    if (bCore) {
        const auto iter = mCoreHandleMap.find(path);
        return iter != mCoreHandleMap.end() ? iter->second : nullptr;
    }

    const auto iter = mExtendHandleMap.find(path);
    return iter != mExtendHandleMap.end() ? iter->second : nullptr;
}

bool UServiceModule::OnServiceShutdown(const std::string &filename, const int32_t sid, const bool bCore) {
    if (GetState() != EModuleState::RUNNING)
        return true;

    if (bCore) {
        // TODO
        return true;
    } else {
        std::unique_lock lock(mFileNameMutex);
        if (const auto iter = mFilenameMapping.find(filename); iter != mFilenameMapping.end()) {
            iter->second.erase(sid);

            if (!iter->second.empty()) {
                return false;
            }

            mFilenameMapping.erase(iter);
            return true;
        }
        return true;
    }
}
