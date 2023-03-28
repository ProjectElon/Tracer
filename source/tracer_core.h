#pragma once

#include "tracer_intrinsics.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

#define function static
#define global_variable static
#define local_persist static

#if TRACER_ASSERTIONS

#define Assert(Expression) \
if ((Expression)) \
{ \
} \
else \
{ \
    fprintf(stderr, "Assertion Failed: %s at %s:%d\n", #Expression, __FILE__, __LINE__); \
    LineBreak(); \
}

#else

#define Assert(Expression)

#endif
