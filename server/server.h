#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define SERVER_API __declspec(dllexport)
#else
    #define SERVER_API __attribute__((visibility("default")))
#endif