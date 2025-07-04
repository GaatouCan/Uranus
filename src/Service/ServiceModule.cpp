#include "ServiceModule.h"
#include "Service.h"
#include "../Package.h"
#include "../Config/Config.h"
#include "../LibraryNode.h"

#include <spdlog/spdlog.h>


UServiceModule::UServiceModule(UServer *server)
    : Super(server),
      mNextID(1) {
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
        if (auto node = new FLibraryNode(); node->LoadFrom(path.string())) {
            mCoreLibraries[filename] = node;
            SPDLOG_INFO("{:<20} - Load Core Service Library[{}] Success", __FUNCTION__, path.string());
        }
    }
    SPDLOG_INFO("Load Core Service Completed!");

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
        if (auto *node = new FLibraryNode(); node->LoadFrom(path.string())) {
            mExtendLibraries[filename] = node;
            SPDLOG_INFO("{:<20} - Load Extend Service Library[{}] Success", __FUNCTION__, path.string());
        }
    }
    SPDLOG_INFO("Load Extend Service Completed!");

    for (const auto &[filename, node] : mCoreLibraries) {
        const int32_t sid = mNextID++;
        // if (mRecycledID.empty()) {
        //     sid = mNextID++;
        // } else {
        //     sid = mRecycledID.front();
        //     mRecycledID.pop();
        // }

        const auto context = std::make_shared<UContext>();

        context->SetUpModule(this);
        context->SetUpLibraryNode(node);
        context->SetServiceID(sid);

        if (context->Initial(nullptr)) {
            mServiceMap[sid] = context;
            mCoreFileNameToID[filename] = sid;

            FContextInfo info {sid, filename, context->GetServiceName(), true};

            mNameToID[info.name] = info;
            mInfoMap[sid] = info;
        } else {
            // context->ForceShutdown();
            // mRecycledID.push(sid);
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
            SPDLOG_ERROR("Failed To Start Core Service[{}]!", context->GetServiceName());
            // context->ForceShutdown();

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

    for (const auto &node : mCoreLibraries | std::views::values) {
        delete node;
    }

    for (const auto &node : mExtendLibraries | std::views::values) {
        delete node;
    }

    SPDLOG_INFO("Unload All Service Success");
}

std::shared_ptr<UContext> UServiceModule::BootExtendService(const std::string &filename, const std::shared_ptr<IPackage> &pkg) {
    if (mState != EModuleState::RUNNING)
        return nullptr;

    const auto node = FindLibrary(filename);
    if (node == nullptr) {
        SPDLOG_WARN("{:<20} - Fail To Found Service Library {}", __FUNCTION__, filename);
        return nullptr;
    }

    const int32_t sid = AllocateServiceID();
    if (sid < 0)
        return nullptr;

    auto context = std::make_shared<UContext>();

    context->SetUpModule(this);
    context->SetUpLibraryNode(node);
    context->SetServiceID(sid);

    if (!context->Initial(pkg)) {
        SPDLOG_ERROR("{:<20} - Failed To Initial Service[{}]", __FUNCTION__, filename);

        context->ForceShutdown();
        RecycleServiceID(sid);

        return nullptr;
    }

    // Check If Service Name Unique
    bool bSuccess = true;
    {
        std::shared_lock lock(mNameMutex);
        if (mNameToID.contains(context->GetServiceName())) {
            SPDLOG_WARN("{:<20} - Service[{}] Already Loaded.", __FUNCTION__, context->GetServiceName());
            bSuccess = false;
        }
    }

    // If Service Name Unique
    if (bSuccess) {
        if (context->BootService()) {
            {
                const FContextInfo info {sid, filename, context->GetServiceName(), false};

                std::scoped_lock lock(mServiceMutex, mFileNameMutex, mNameMutex);
                mServiceMap[sid] = context;
                mExtendFileNameToID[filename].insert(sid);

                mNameToID[info.name] = info;
                mInfoMap[sid] = info;
            }

            SPDLOG_INFO("{:<20} - Boot Extend Service[{}] Successfully", __FUNCTION__, context->GetServiceName());
            return context;
        }

        SPDLOG_ERROR("{:<20} - Failed To Boot Extend Service[{}]", __FUNCTION__, context->GetServiceName());
    } else {
        SPDLOG_ERROR("{:<20} - Extend Service[{}] Has Already Existed", __FUNCTION__, context->GetServiceName());
    }

    // If Not Unique, Service Force To Shut Down And Recycle The Service ID
    context->ForceShutdown();
    RecycleServiceID(sid);

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

    } else {
        {
            std::scoped_lock lock(mServiceMutex, mNameMutex);
            if (const auto &iter = mServiceMap.find(id); iter != mServiceMap.end()) {
                context = iter->second;
                mServiceMap.erase(iter);
            }

            mInfoMap.erase(id);
            mNameToID.erase(info.name);
        }

        if (context == nullptr) {
            RecycleServiceID(id);
            return;
        }

        auto func = [this, id, filename = info.filename](IContext *) {
            if (mState != EModuleState::RUNNING)
                return;
            OnServiceShutdown(filename, id, false);
            RecycleServiceID(id);
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
        std::shared_lock lock(mNameMutex);
        const auto nameIter = mNameToID.find(name);
        if (nameIter == mNameToID.end())
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

    std::shared_lock lock(mNameMutex);
    std::map<std::string, int32_t> result;
    for (const auto &[fst, snd] : mNameToID) {
        result.emplace(fst, snd.id);
    }
    return result;
}

int32_t UServiceModule::GetServiceID(const std::string &name) const {
    if (mState != EModuleState::INITIALIZED || mState != EModuleState::RUNNING)
        return -10;

    std::shared_lock lock(mNameMutex);
    const auto nameIter = mNameToID.find(name);
    return nameIter != mNameToID.end() ? nameIter->second.id : -10;
}

void UServiceModule::LoadLibraryFrom(const std::string &path, const bool bCore) {
    if (mState != EModuleState::RUNNING)
        return;

    if (bCore) {
        // TODO:
    } else {
        std::unique_lock lock(mLibraryMutex);
        if (mExtendLibraries.contains(path)) {
            SPDLOG_INFO("{:<20} - Library From {} Exists.", __FUNCTION__, path);
            return;
        }

        if (const auto node = new FLibraryNode(); node->LoadFrom(path)) {
            mExtendLibraries[path] = node;
            SPDLOG_INFO("{:<20} - Load Extend Library From {} Success.", __FUNCTION__, path);
        }
    }
}

void UServiceModule::UnloadLibrary(const std::string &path, const bool bCore) {
    if (mState != EModuleState::RUNNING)
        return;

    if (bCore) {
        // TODO: Core Service Not Implement Now
    } else {
        SPDLOG_INFO("{:<20} - Begin Unload Extend Service[{}] Library...", __FUNCTION__, path);

        absl::flat_hash_set<int32_t> del;
        bool bComplete = true;

        {
            std::unique_lock lock(mFileNameMutex);
            if (const auto iter = mExtendFileNameToID.find(path); iter != mExtendFileNameToID.end()) {
                del = iter->second;
            }
        }

        for (const auto &sid : del) {
            std::shared_ptr<UContext> context;
            FContextInfo info;

            {
                std::scoped_lock lock(mServiceMutex, mNameMutex);
                if (const auto iter = mServiceMap.find(sid); iter != mServiceMap.end()) {
                    context = iter->second;
                    mServiceMap.erase(iter);
                }

                if (const auto iter = mInfoMap.find(sid); iter != mInfoMap.end()) {
                    info = iter->second;
                    mInfoMap.erase(iter);
                    mNameToID.erase(info.name);
                }
            }

            auto func = [this, path, sid](IContext *context) {
                if (mState != EModuleState::RUNNING)
                    return;

                const bool bCanDelete = OnServiceShutdown(path, sid, false);
                RecycleServiceID(sid);

                if (bCanDelete) {
                    if (const auto iter = mExtendLibraries.find(path); iter != mExtendLibraries.end()) {
                        delete iter->second;
                        mExtendLibraries.erase(iter);
                    }
                }
            };

            if (context != nullptr) {
                if (context->Shutdown(false, 5, func) == 0) {
                    bComplete = false;
                    SPDLOG_INFO("{:<20} - Extend Service[{} - {}] Waiting Complete", __FUNCTION__, sid, context->GetServiceName());
                } else {
                    SPDLOG_INFO("{:<20} - Extend Service[{}] Shutdown", __FUNCTION__, sid);
                }
            }
        }

        if (bComplete) {
            {
                std::unique_lock lock(mFileNameMutex);
                if (const auto iter = mExtendFileNameToID.find(path); iter != mExtendFileNameToID.end()) {
                    mExtendFileNameToID.erase(iter);
                }
            }
            SPDLOG_INFO("{:<20} - All Extend Service[{}] Shutdown", __FUNCTION__, path);

            std::unique_lock idLock(mIDMutex);
            for (const auto &sid : del) {
                mRecycledID.push(sid);
            }
        }
    }
}

UServiceModule::FContextInfo UServiceModule::GetContextInfo(int32_t id) const {
    if (GetState() != EModuleState::RUNNING)
        return {};

    std::shared_lock lock(mNameMutex);
    const auto iter = mInfoMap.find(id);
    return iter == mInfoMap.end() ? FContextInfo{} : iter->second;
}

FLibraryNode *UServiceModule::FindLibrary(const std::string &path, const bool bCore) const {
    std::shared_lock lock(mLibraryMutex);
    if (bCore) {
        const auto iter = mCoreLibraries.find(path);
        return iter != mCoreLibraries.end() ? iter->second : nullptr;
    }

    const auto iter = mExtendLibraries.find(path);
    return iter != mExtendLibraries.end() ? iter->second : nullptr;
}

int32_t UServiceModule::AllocateServiceID() {
    std::unique_lock lock(mIDMutex);

    if (!mRecycledID.empty()) {
        const int32_t id = mRecycledID.front();
        mRecycledID.pop();
        return id;
    }

    if (mNextID < 0) {
        SPDLOG_ERROR("{:<20} - Service ID Allocator Overflow", __FUNCTION__);
        return -1;
    }

    return mNextID++;
}

void UServiceModule::RecycleServiceID(const int32_t id) {
    std::unique_lock lock(mIDMutex);
    mRecycledID.push(id);
}

bool UServiceModule::OnServiceShutdown(const std::string &filename, const int32_t sid, const bool bCore) {
    if (GetState() != EModuleState::RUNNING)
        return true;

    if (bCore) {
        // TODO
        return true;
    } else {
        std::unique_lock lock(mFileNameMutex);
        if (const auto iter = mExtendFileNameToID.find(filename); iter != mExtendFileNameToID.end()) {
            iter->second.erase(sid);

            if (!iter->second.empty()) {
                return false;
            }

            mExtendFileNameToID.erase(iter);
            return true;
        }
        return true;
    }
}
