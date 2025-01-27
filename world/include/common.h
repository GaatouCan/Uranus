#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define BASE_API __declspec(dllexport)
#else
    #define BASE_API __attribute__((visibility("default")))
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define PLUGIN_API __declspec(dllexport)
#else
    #define PLUGIN_API __attribute__((visibility("default")))
#endif

#define DISABLE_COPY(clazz) \
    clazz(const clazz&) = delete; \
    clazz &operator=(const clazz&) = delete; \

#define DISABLE_MOVE(clazz) \
    clazz(clazz &&) noexcept = delete; \
    clazz &operator=(clazz &&) noexcept = delete;

#define DISABLE_COPY_MOVE(clazz) \
    DISABLE_COPY(clazz) \
    DISABLE_MOVE(clazz)
