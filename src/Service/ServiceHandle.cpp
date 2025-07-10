#include "../ServiceHandle.h"

#include <spdlog/spdlog.h>


FServiceHandle::FServiceHandle()
    : mHandle(nullptr),
      mCreator(nullptr),
      mDestroyer(nullptr) {
}

FServiceHandle::~FServiceHandle() {
    Unload();
}

void FServiceHandle::Reset() {
    mHandle = nullptr;
    mCreator = nullptr;
    mDestroyer = nullptr;
    mPath.clear();
}

bool FServiceHandle::LoadFrom(const std::string &path) {
#if defined(_WIN32) || defined(_WIN64)
    mHandle = LoadLibrary(path.data());

    if (mHandle == nullptr) {
        SPDLOG_ERROR("{:<20} - Failed To Load Service From {}", __FUNCTION__, path);
        Reset();
        return false;
    }

    mCreator = GetProcAddress(mHandle, "NewService");
    mDestroyer = GetProcAddress(mHandle, "DestroyService");

    if (mCreator == nullptr || mDestroyer == nullptr) {
        SPDLOG_ERROR("{:<20} - Failed To Find Service Creator Or Destroyer, From {}", __FUNCTION__, path.data());
        FreeLibrary(mHandle);
        Reset();
        return false;
    }

#else
    handle_ = dlopen(path.data(), RTLD_LAZY);

    if (handle_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Failed To Load Service {}", __FUNCTION__, path.data());
        Reset();
        return false;
    }

    creator_ = dlsym(handle, "NewService");
    destroyer_ = dlsym(handle, "DestroyService");

    if (creator_ == nullptr || destroyer_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Failed To Find Service Creator Or Destroyer {}", __FUNCTION__, path.data());
        dlclose(handle_);
        Reset();
        return false;
    }

#endif

    mPath = path;
    SPDLOG_INFO("{:<20} - Loaded Service From[{}] Success", __FUNCTION__, path);

    return true;
}

void FServiceHandle::Unload() {
    if (mHandle == nullptr)
        return;

#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary(mHandle);
#else
    dlclose(handle);
#endif

    SPDLOG_INFO("{:<20} - Unloaded Service[{}] Success", __FUNCTION__, mPath);
    Reset();
}

void *FServiceHandle::GetCreator() const {
    return mCreator;
}

void *FServiceHandle::GetDestroyer() const {
    return mDestroyer;
}

std::string FServiceHandle::GetPath() const {
    return mPath;
}
