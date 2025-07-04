#include "LibraryNode.h"

#include <spdlog/spdlog.h>


FLibraryNode::FLibraryNode()
    : mHandle(nullptr),
      mCreator(nullptr),
      mDestroyer(nullptr) {
}

FLibraryNode::~FLibraryNode() {
    Unload();
}

void FLibraryNode::Reset() {
    mHandle = nullptr;
    mCreator = nullptr;
    mDestroyer = nullptr;
    mPath.clear();
}

bool FLibraryNode::LoadFrom(const std::string &path) {
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

void FLibraryNode::Unload() {
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

void *FLibraryNode::GetCreator() const {
    return mCreator;
}

void *FLibraryNode::GetDestroyer() const {
    return mDestroyer;
}

std::string FLibraryNode::GetPath() const {
    return mPath;
}
