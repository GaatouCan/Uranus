#include "LibraryHandle.h"

#include <spdlog/spdlog.h>


FLibraryHandle::FLibraryHandle()
    : handle_(nullptr),
      creator_(nullptr),
      destroyer_(nullptr) {
}

FLibraryHandle::~FLibraryHandle() {
    Unload();
}

void FLibraryHandle::Reset() {
    handle_ = nullptr;
    creator_ = nullptr;
    destroyer_ = nullptr;
    path_.clear();
}

bool FLibraryHandle::LoadFrom(const std::string &path) {
#if defined(_WIN32) || defined(_WIN64)
    handle_ = LoadLibrary(path.data());

    if (handle_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Failed To Load Library From[{}]", __FUNCTION__, path);
        Reset();
        return false;
    }

    creator_ = GetProcAddress(handle_, "CreateInstance");
    destroyer_ = GetProcAddress(handle_, "DestroyInstance");

    if (creator_ == nullptr || destroyer_ == nullptr) {
        SPDLOG_ERROR("{:<20} - Cannot Find Creator Or Destroyer From Library[{}]", __FUNCTION__, path);
        FreeLibrary(handle_);
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

    path_ = path;
    SPDLOG_INFO("{:<20} - Successfully Loaded Dynamic Library From[{}]", __FUNCTION__, path_);

    return true;
}

void FLibraryHandle::Unload() {
    if (handle_ == nullptr)
        return;

#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary(handle_);
#else
    dlclose(handle);
#endif

    SPDLOG_INFO("{:<20} - Successfully Released Dynamic Library[{}]", __FUNCTION__, path_);
    Reset();
}

void *FLibraryHandle::GetCreator() const {
    return creator_;
}

void *FLibraryHandle::GetDestroyer() const {
    return destroyer_;
}

std::string FLibraryHandle::GetPath() const {
    return path_;
}
