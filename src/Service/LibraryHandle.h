#pragma once

#include "Utils.h"

/**
 * Used To Load Service Dynamic Library On Windows Or Linux,
 * The Handle Will Be Closed While Call Destructor
 */
class BASE_API FLibraryHandle final {

    AModuleHandle mHandle;

    void *mCreator;
    void *mDestroyer;

    std::string mPath;

public:
    FLibraryHandle();
    ~FLibraryHandle();

    void Reset();

    bool LoadFrom(const std::string &path);
    void Unload();

    [[nodiscard]] void *GetCreator() const;
    [[nodiscard]] void *GetDestroyer() const;

    [[nodiscard]] std::string GetPath() const;

    template<typename FuncType>
    FuncType GetCreatorT() const {
        return reinterpret_cast<FuncType>(mCreator);
    }

    template<typename FuncType>
    FuncType GetDestroyerT() const {
        return reinterpret_cast<FuncType>(mDestroyer);
    }
};


