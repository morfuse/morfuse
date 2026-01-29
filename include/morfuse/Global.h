#pragma once

#include <cstdint>

#undef mfuse_EXPORTS

#ifdef mfuse_DLL
    #ifdef __GNUC__
        #define mfuse_EXPORTS __attribute__ ((visibility ("default")))
        #define mfuse_PUBLIC __attribute__ ((visibility ("default")))
        #define mfuse_LOCAL __attribute__ ((visibility ("hidden")))
    #else
        #define mfuse_EXPORTS __declspec(dllexport)
        #define mfuse_PUBLIC
        #define mfuse_LOCAL
    #endif
    #define mfuse_TEMPLATE
#else
    #ifdef __GNUC__
        #define mfuse_EXPORTS
        #define mfuse_PUBLIC
        #define mfuse_LOCAL
    #else
        #define mfuse_EXPORTS __declspec(dllimport)
        #define mfuse_PUBLIC
        #define mfuse_LOCAL
    #endif
    #define mfuse_TEMPLATE extern
#endif
