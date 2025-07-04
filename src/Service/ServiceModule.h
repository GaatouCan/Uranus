#pragma once

#include "../Module.h"

#include <queue>
#include <shared_mutex>
#include <memory>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>


class IPackage;
class UContext;
class IService;
class FLibraryNode;

using absl::flat_hash_map;
using absl::flat_hash_set;


class BASE_API UServiceModule final : public IModule {

    DECLARE_MODULE(UServiceModule)

    struct FContextInfo {
        int32_t id = INVALID_SERVICE_ID;
        std::string filename;
        std::string name;
        bool bCore = false;
    };

protected:
    explicit UServiceModule(UServer *server);

    void Initial() override;
    void Start() override;
    void Stop() override;

public:
    ~UServiceModule() override;

    constexpr const char *GetModuleName() const override {
        return "Service Module";
    }

    std::shared_ptr<UContext> BootExtendService(const std::string &filename, const std::shared_ptr<IPackage> &pkg = nullptr);

    void ShutdownService(int32_t id);

    std::shared_ptr<UContext> FindService(int32_t id) const;
    std::shared_ptr<UContext> FindService(const std::string &name) const;

    std::map<std::string, int32_t> GetServiceList() const;
    int32_t GetServiceID(const std::string &name) const;

    void LoadLibraryFrom(const std::string &path, bool bCore = false);
    void UnloadLibrary(const std::string &path, bool bCore = false);

private:
    FContextInfo GetContextInfo(int32_t id) const;
    FLibraryNode *FindLibrary(const std::string &path, bool bCore = false) const;

    int32_t AllocateServiceID();
    void RecycleServiceID(int32_t id);

    bool OnServiceShutdown(const std::string &filename, int32_t sid, bool bCore = false);

private:
    flat_hash_map<std::string, FLibraryNode *> mExtendLibraries;
    flat_hash_map<std::string, FLibraryNode *> mCoreLibraries;
    mutable std::shared_mutex mLibraryMutex;

    flat_hash_map<int32_t, std::shared_ptr<UContext>> mServiceMap;
    mutable std::shared_mutex mServiceMutex;

    flat_hash_map<std::string, FContextInfo> mNameToID;
    flat_hash_map<int32_t, FContextInfo> mInfoMap;
    mutable std::shared_mutex mNameMutex;

    flat_hash_map<std::string, int32_t> mCoreFileNameToID;
    flat_hash_map<std::string, flat_hash_set<int32_t>> mExtendFileNameToID;
    mutable std::shared_mutex mFileNameMutex;

    // Service ID Management

    std::queue<int32_t> mRecycledID;
    int32_t mNextID;
    mutable std::shared_mutex mIDMutex;
};
