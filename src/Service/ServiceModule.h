#pragma once

#include "Module.h"

#include <queue>
#include <shared_mutex>
#include <memory>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>


class IPackageInterface;
class UContext;
class IServiceBase;
class FLibraryHandle;

using absl::flat_hash_map;
using absl::flat_hash_set;


class BASE_API UServiceModule final : public IModuleBase {

    DECLARE_MODULE(UServiceModule)

    struct FServiceInfo {
        int32_t id = INVALID_SERVICE_ID;
        std::string filename;
        std::string name;
        bool bCore = false;
    };

protected:
    UServiceModule();

    void Initial() override;
    void Start() override;
    void Stop() override;

public:
    ~UServiceModule() override;

    constexpr const char *GetModuleName() const override {
        return "Service Module";
    }

    std::shared_ptr<UContext> BootExtendService(const std::string &filename, const std::shared_ptr<IPackageInterface> &pkg = nullptr);

    void ShutdownService(int32_t id);

    std::shared_ptr<UContext> FindService(int32_t id) const;
    std::shared_ptr<UContext> FindService(const std::string &name) const;

    std::map<std::string, int32_t> GetServiceList() const;
    int32_t GetServiceID(const std::string &name) const;

    void LoadLibraryFrom(const std::string &filename, bool bCore = false);
    void UnloadLibrary(const std::string &filename, bool bCore = false);

private:
    FServiceInfo GetContextInfo(int32_t id) const;
    FLibraryHandle *FindServiceHandle(const std::string &path, bool bCore = false) const;

    int32_t AllocateServiceID();
    void RecycleServiceID(int32_t id);

    bool OnServiceShutdown(const std::string &filename, int32_t sid, bool bCore = false);

private:
    /** Dynamic Library Handle **/

    flat_hash_map<std::string, FLibraryHandle *> extendHandleMap_;
    flat_hash_map<std::string, FLibraryHandle *> coreHandleMap_;
    mutable std::shared_mutex handleMutex_;

    /** Running Services Map **/
    flat_hash_map<int32_t, std::shared_ptr<UContext>> serviceMap_;
    mutable std::shared_mutex serviceMutex_;

    /** Service Name To Service ID Mapping **/
    flat_hash_map<std::string, FServiceInfo> serviceInfoMap_;
    mutable std::shared_mutex infoMutex_;

    /** Service ID Set With Same Library Filename **/
    flat_hash_map<std::string, flat_hash_set<int32_t>> filenameMapping_;
    mutable std::shared_mutex fileNameMutex_;


    /** Service ID Management **/

    std::queue<int32_t> recycledId_;
    int32_t nextId_;
    mutable std::shared_mutex idMutex_;
};
