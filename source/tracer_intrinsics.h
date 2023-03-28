#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#include <immintrin.h>
#endif

inline void LineBreak()
{
#ifdef _MSC_VER
    __debugbreak();
#else
    #error "unsupported compiler"
#endif
}