#pragma once

#include "tracer_core.h"
#include "tracer_intrinsics.h"

#include <math.h>
#include <float.h>

#define PI 3.14159265358979323846f
#define Two_PI 6.28318530718f
#define PI_OVER_180_DEGREES 0.01745329251f
#define DEG_TO_RAD PI_OVER_180_DEGREES
#define MAX_F32 FLT_MAX
#define MIN_F32 FLT_MIN

#define Lerp(A, B, T) ((A) * (1.0f - (T)) + (B) * (T))

inline f32 Radians(f32 Degrees)
{
    return Degrees * DEG_TO_RAD;
}

inline u32 GetPixelIndex(u32 X, u32 Y, u32 Width)
{
    return Y * Width + X;
}

inline bool
NearZero(f32 Scalar, f32 Epsilion = 0.0000001f)
{
    return fabsf(Scalar) < Epsilion;
}

struct color8
{
    u8 R;
    u8 G;
    u8 B;
};

struct v3
{
    f32 X;
    f32 Y;
    f32 Z;
};

struct v4
{
    f32 X;
    f32 Y;
    f32 Z;
    f32 W;
};

inline v3 V3(f32 X, f32 Y, f32 Z)
{
    v3 Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    return Result;
}

inline v3 V3(f32 Scalar)
{
    v3 Result;
    Result.X = Scalar;
    Result.Y = Scalar;
    Result.Z = Scalar;
    return Result;
}

inline v3 operator+(const v3 &A, const v3 &B)
{
    v3 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    Result.Z = A.Z + B.Z;
    return Result;
}

inline v3& operator+=(v3 &A, const v3 &B)
{
    A.X += B.X;
    A.Y += B.Y;
    A.Z += B.Z;
    return A;
}

inline v3 operator-(const v3 &V)
{
    v3 Result;
    Result.X = -V.X;
    Result.Y = -V.Y;
    Result.Z = -V.Z;
    return Result;
}

inline v3 operator-(const v3 &A, const v3 &B)
{
    v3 Result = {};
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;
    return Result;
}

inline v3 operator*(const v3 &V, f32 Scalar)
{
    v3 Result;
    Result.X = V.X * Scalar;
    Result.Y = V.Y * Scalar;
    Result.Z = V.Z * Scalar;
    return Result;
}

inline v3 operator*(f32 Scalar, const v3 &V)
{
    v3 Result;
    Result.X = V.X * Scalar;
    Result.Y = V.Y * Scalar;
    Result.Z = V.Z * Scalar;
    return Result;
}

inline v3 operator/(const v3 &V, f32 Scalar)
{
    v3 Result;
    Result.X = V.X / Scalar;
    Result.Y = V.Y / Scalar;
    Result.Z = V.Z / Scalar;
    return Result;
}

inline v3 operator/=(v3 &V, f32 Scalar)
{
    f32 OneOverSalar = 1.0f / Scalar;
    V.X *= OneOverSalar;
    V.Y *= OneOverSalar;
    V.Z *= OneOverSalar;
    return V;
}

inline f32 Length(const v3 &V)
{
    return sqrtf(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
}

inline v3 Abs(const v3 &V)
{
    v3 Result;
    Result.X = fabsf(V.X);
    Result.Y = fabsf(V.Y);
    Result.Z = fabsf(V.Z);
    return Result;
}

inline f32 Dot(const v3 &A, const v3 &B)
{
    return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}

inline v3 Reflect(const v3& V, const v3& Normal)
{
    return V - 2.0f * Dot(V, Normal) * Normal;
}

// todo(harlequin): are the branchless versions actually faster ?
inline f32 Minimum(f32 A, f32 B)
{
    f32 Selection = (f32)(A <= B);
    return A * Selection + (1.0f - Selection) * B;
}

inline f32 Maximium(f32 A, f32 B)
{
    f32 Selection = (f32)(A >= B);
    return A * Selection + (1.0f - Selection) * B;
}

inline f32 Clamp(f32 Value, f32 MinValue, f32 MaxValue)
{
    return Minimum(Maximium(Value, MinValue), MaxValue);
}

inline v3 Clamp(const v3 &V,
                const v3 &MinValues,
                const v3 &MaxValues)
{
    v3 Result;

    Result.X = Clamp(V.X, MinValues.X, MaxValues.X);
    Result.Y = Clamp(V.Y, MinValues.Y, MaxValues.Y);
    Result.Z = Clamp(V.Z, MinValues.Z, MaxValues.Z);

    return Result;
}

function v3
Normalize(const v3 &V,
          const v3 &ReturnedV3IfVLengthIsZero = { 0.0f, 0.0f, 0.0f });

inline color8 NormalizedColorToColor8(const v3 &NormalizedColor)
{
    color8 Result;
    Result.R = (u8)(NormalizedColor.X * 255.0f);
    Result.G = (u8)(NormalizedColor.Y * 255.0f);
    Result.B = (u8)(NormalizedColor.Z * 255.0f);
    return Result;
}

v4 V4(const v3& V, f32 W = 0.0f)
{
    v4 Result;
    Result.X = V.X;
    Result.Y = V.Y;
    Result.Z = V.Z;
    Result.W = W;
    return Result;
}

struct ray
{
    v3 Origin;
    v3 Direction;

    __m128 _Origin;
    __m128 _Direction;
};

inline ray
RayOriginDirection(const v3 &Origin, const v3 &Direction)
{
    ray Result;
    Result.Origin    = Origin;
    Result.Direction = Direction;
    Result._Origin = _mm_load_ps((const f32*)&V4(Origin));
    Result._Direction = _mm_load_ps((const f32*)&V4(Direction));
    return Result;
}

inline v3
SampleRay(const ray &Ray, f32 T)
{
    v3 Result = Ray.Origin + Ray.Direction * T;
    return Result;
}

struct sphere
{
    v3  Center;
    f32 Radius;

    __m128 _Center;
};

inline sphere
SphereCenterRadius(const v3 &Center, f32 Radius)
{
    sphere Result;
    Result.Center = Center;
    Result.Radius = Radius;

    Result._Center = _mm_load_ps((const f32*)&V4(Center));
    return Result;
}

struct intersection_info
{
    v3   Point;
    v3   Normal;
    bool FrontFace;
};

function intersection_info
GetRaySphereIntersectionInfo(const ray    &Ray,
                             const sphere &Sphere,
                             f32           T);

inline v3
NormalToNormalizedColor(const v3 &Normal)
{
    return (Normal + V3(1.0f)) * 0.5f;
}