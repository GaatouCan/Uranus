#pragma once

#include "Module.h"

#include <queue>
#include <shared_mutex>
#include <memory>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>


class IPackage;
class UContext;
class IService;
class FLibraryHandle;

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

    void LoadLibraryFrom(const std::string &filename, bool bCore = false);
    void UnloadLibrary(const std::string &filename, bool bCore = false);

private:
    FContextInfo GetContextInfo(int32_t id) const;
    FLibraryHandle *FindServiceHandle(const std::string &path, bool bCore = false) const;

    int32_t AllocateServiceID();
    void RecycleServiceID(int32_t id);

    bool OnServiceShutdown(const std::string &filename, int32_t sid, bool bCore = false);

private:
    /** Dynamic Library Handle **/

    flat_hash_map<std::string, FLibraryHandle *> mExtendHandleMap;
    flat_hash_map<std::string, FLibraryHandle *> mCoreHandleMap;
    mutable std::shared_mutex mHandleMutex;

    /** Running Services Map **/
    flat_hash_map<int32_t, std::shared_ptr<UContext>> mServiceMap;
    mutable std::shared_mutex mServiceMutex;

    /** Service Name To Service ID Mapping **/
    flat_hash_map<std::string, FContextInfo> mContextInfoMap;
    mutable std::shared_mutex mNameMutex;

    /** Service ID Set With Same Library Filename **/
    flat_hash_map<std::string, flat_hash_set<int32_t>> mFilenameToServiceID;
    mutable std::shared_mutex mFileNameMutex;


    /** Service ID Management **/

    std::queue<int32_t> mRecycledID;
    int32_t mNextID;
    mutable std::shared_mutex mIDMutex;
};
