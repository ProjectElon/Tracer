#pragma once

#include "tracer_core.h"
#include "tracer_math.h"

#include <time.h>

struct random_series
{
    u64 ShuffleTable[2];
};

inline random_series
RandomSeries()
{
    random_series Series = {};
    Series.ShuffleTable[0] = time(nullptr);
    Series.ShuffleTable[1] = time(nullptr);
    return Series;
}

inline f32 RandomCanonical(random_series *Series)
{
    u64 Seed0  = Series->ShuffleTable[1];
    u64 Seed1  = Series->ShuffleTable[0];
    u64 Result = Seed0 + Seed1;
    Series->ShuffleTable[0] = Seed0;
    Seed1 ^= Seed1 << 23;
    Series->ShuffleTable[1] = Seed1 ^ Seed0 ^ (Seed1 >> 18) ^ (Seed0 >> 5);
    return f32((f64(Result) / f64(ULLONG_MAX)));
}

inline f32 RandomBetween(random_series *Series,
                         f32            MinValue,
                         f32            MaxValue)
{
    f32 RandomValue = RandomCanonical(Series);
    return MinValue + (MaxValue - MinValue) * RandomValue;
}

inline v3 RandomV3(random_series *Series,
                   f32            MinValue,
                   f32            MaxValue)
{
    v3 Result = V3(RandomBetween(Series, MinValue, MaxValue),
                   RandomBetween(Series, MinValue, MaxValue),
                   RandomBetween(Series, MinValue, MaxValue));
    return Result;
}

inline v3 RandomUnitV3(random_series *Series)
{
    f32 Phi      = RandomBetween(Series, 0.0f, Two_PI);
    f32 Theta    = RandomBetween(Series, 0.0f, Two_PI);
    f32 SinPhi   = sinf(Phi);
    f32 CosPhi   = cosf(Phi);
    f32 SinTheta = sinf(Theta);
    f32 CosTheta = cosf(Theta);
    v3 Result = V3(CosPhi * SinTheta,
                   SinPhi * SinTheta,
                   CosTheta);
    return Result;
}

inline v3 RandomInHemisphere(random_series *Series,
                             const v3      &Normal)
{
    v3 RandomDirection = RandomUnitV3(Series);
    if (Dot(RandomDirection, Normal) > 0.0f)
    {
        return RandomDirection;
    }
    return -RandomDirection;
}