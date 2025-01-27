#pragma once

/**
 * 这个文件可以用来写一些业务层常用的封装
 * 尽量使用inline关键字
 * 由于可能会包含很多头文件 所以务必只在源文件中调用
 */
#include "ProtoType.h"

#include <system/scene/Scene.h>
#include <impl/Package.h>
#include <PackagePool.h>

using namespace protocol;

inline void SceneBroadcast(UScene *scene, EProtoType type, const std::string_view data,  const std::set<uint32_t> &except = {}) {
    if (scene == nullptr)
        return;

    FPackage pkg;
    UPackagePool::InitPackage(&pkg);
    pkg.SetPackageID(static_cast<uint32_t>(type)).SetData(data);

    scene->BroadCast(&pkg, except);
}