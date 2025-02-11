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
struct BASE_API UniqueID {

    long long time;
    int64_t random;

    [[nodiscard]] std::string ToString() const;

    UniqueID &FromString(const std::string &str);

    static UniqueID RandomGenerate();

    bool operator<(const UniqueID &other) const;

    bool operator==(const UniqueID& other) const;
    bool operator!=(const UniqueID& other) const;
};
