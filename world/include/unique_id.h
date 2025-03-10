#pragma once

#include "common.h"

#include <string>

#ifdef __linux__
#include <cstdint>
#endif


/**
 * 包含一个当前时间戳和一个随机数 唯一性很高
 * 但避免随机数一样 建议生成的时候做while循环比对
 */
struct BASE_API FUniqueID {

    long long time;
    int64_t random;

    [[nodiscard]] std::string toString() const;

    FUniqueID &fromString(const std::string &str);

    static FUniqueID randomGenerate();

    bool operator<(const FUniqueID &other) const;

    bool operator==(const FUniqueID& other) const;
    bool operator!=(const FUniqueID& other) const;
};
