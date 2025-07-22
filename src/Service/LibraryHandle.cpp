#include "LibraryHandle.h"

#include <spdlog/spdlog.h>


FLibraryHandle::FLibraryHandle()
    : mHandle(nullptr),
      mCreator(nullptr),
      mDestroyer(nullptr) {
}

FLibraryHandle::~FLibraryHandle() {
    Unload();
}

void FLibraryHandle::Reset() {
    mHandle = nullptr;
    mCreator = nullptr;
    mDestroyer = nullptr;
    mPath.clear();
}

bool FLibraryHandle::LoadFrom(const std::string &path) {
#if defined(_WIN32) || defined(_WIN64)
    mHandle = LoadLibrary(path.data());

    if (mHandle == nullptr) {
        SPDLOG_ERROR("{:<20} - Failed To Load Library From[{}]", __FUNCTION__, path);
        Reset();
        return false;
    }

    mCreator = GetProcAddress(mHandle, "CreateInstance");
    mDestroyer = GetProcAddress(mHandle, "DestroyInstance");

    if (mCreator == nullptr || mDestroyer == nullptr) {
        SPDLOG_ERROR("{:<20} - Cannot Find Creator Or Destroyer From Library[{}]", __FUNCTION__, path);
        FreeLibrary(mHandle);
        Reset();
        return false;
    }

#else
    handle_ = dlopen(path.data(), RTLD_LAZY);

    if (handle_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Failed To Load Library From[{}]", __FUNCTION__, path);
        Reset();
        return false;
    }

    creator_ = dlsym(handle, "CreateInstance");
    destroyer_ = dlsym(handle, "DestroyInstance");

    if (creator_ == nullptr || destroyer_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Cannot Find Creator Or Destroyer From Library[{}]", __FUNCTION__, path);
        dlclose(handle_);
        Reset();
        return false;
    }

#endif

    mPath = path;
    SPDLOG_INFO("{:<20} - Successfully Loaded Dynamic Library From[{}]", __FUNCTION__, mPath);

    return true;
}

void FLibraryHandle::Unload() {
    if (mHandle == nullptr)
        return;

#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary(mHandle);
#else
    dlclose(handle);
#endif

    SPDLOG_INFO("{:<20} - Successfully Released Dynamic Library[{}]", __FUNCTION__, mPath);
    Reset();
}

void *FLibraryHandle::GetCreator() const {
    return mCreator;
}

void *FLibraryHandle::GetDestroyer() const {
    return mDestroyer;
}

std::string FLibraryHandle::GetPath() const {
    return mPath;
}
