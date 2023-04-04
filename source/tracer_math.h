#pragma once

#include "tracer_core.h"
#include "tracer_intrinsics.h"

#include <math.h>
#include <float.h>

#define ENABLE_SIMD 1
#define PI 3.14159265358979323846f
#define Two_PI 6.28318530718f
#define PI_OVER_180_DEGREES 0.01745329251f
#define DEG_TO_RAD PI_OVER_180_DEGREES
#define MAX_F32 FLT_MAX
#define MIN_F32 FLT_MIN

#define Lerp(A, B, T) ((A) * (1.0f - (T)) + (B) * (T))

function inline f32
Radians(f32 Degrees)
{
    return Degrees * DEG_TO_RAD;
}

function inline u32
GetPixelIndex(u32 X, u32 Y, u32 Width)
{
    return Y * Width + X;
}

function inline bool
NearZero(f32 Scalar, f32 Epsilion = 0.0000001f)
{
    return fabsf(Scalar) < Epsilion;
}

function inline f32
Pow(f32 Base, f32 Exponent)
{
	return powf(Base, Exponent);
}

struct color8
{
	u8 R;
	u8 G;
	u8 B;
};

struct v2
{
	f32 X;
	f32 Y;
};

#define VectorComponent(V, I) ((f32*)(&V))[(I)]

#if ENABLE_SIMD

typedef __m128 v3;
typedef __m128 v4;

#else

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

#endif

function inline v2
V2(f32 X, f32 Y)
{
	v2 Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

function inline v2
operator*(v2 V, f32 Scalar)
{
	v2 Result;
	Result.X = V.X * Scalar;
	Result.Y = V.Y * Scalar;
	return Result;
}

function inline v2
operator*(f32 Scalar, v2 V)
{
	v2 Result;
	Result.X = V.X * Scalar;
	Result.Y = V.Y * Scalar;
	return Result;
}

function inline v3
V3(f32 X, f32 Y, f32 Z)
{
#if ENABLE_SIMD
	v3 Result = _mm_set_ps(0.0f, Z, Y, X);
#else
	v3 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
#endif
	return Result;
}

function inline v3
V3(v2 V, f32 Z)
{
#if ENABLE_SIMD
	v3 Result = _mm_set_ps(0.0f, Z, V.Y, V.X);
#else
	v3 Result;
	Result.X = V.X;
	Result.Y = V.Y;
	Result.Z = Z;
#endif
	return Result;
}

function inline v3
V3(f32 Scalar)
{
#if ENABLE_SIMD
	v3 Result = _mm_set_ps(0.0f, Scalar, Scalar, Scalar);
#else
	v3 Result;
	Result.X = Scalar;
	Result.Y = Scalar;
	Result.Z = Scalar;
#endif
	return Result;
}

function inline v3
operator+(const v3 &A, const v3 &B)
{
#if ENABLE_SIMD
	v3 Result = _mm_add_ps(A, B);
#else
	v3 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	Result.Z = A.Z + B.Z;
#endif
	return Result;
}

function inline v3&
operator+=(v3 &A, const v3 &B)
{
#if ENABLE_SIMD
	A = _mm_add_ps(A, B);
#else
	A.X += B.X;
	A.Y += B.Y;
	A.Z += B.Z;
#endif
	return A;
}

function inline v3
operator-(const v3 &V)
{
#if ENABLE_SIMD
	v3 Result = _mm_sub_ps(_mm_set1_ps(0.0), V);
#else
	v3 Result;
	Result.X = -V.X;
	Result.Y = -V.Y;
	Result.Z = -V.Z;
#endif
	return Result;
}

function inline v3
operator-(const v3 &A, const v3 &B)
{
#if ENABLE_SIMD
	v3 Result = _mm_sub_ps(A, B);
#else
	v3 Result = {};
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	Result.Z = A.Z - B.Z;
#endif
	return Result;
}

function inline v3
operator*(const v3 &V, f32 Scalar)
{
#if ENABLE_SIMD
	v3 Result = _mm_mul_ps(V, _mm_set1_ps(Scalar));
#else
	v3 Result;
	Result.X = V.X * Scalar;
	Result.Y = V.Y * Scalar;
	Result.Z = V.Z * Scalar;
#endif
	return Result;
}

function inline v3
operator*(f32 Scalar, const v3 &V)
{
#if ENABLE_SIMD
	v3 Result = _mm_mul_ps(V, _mm_set1_ps(Scalar));
#else
	v3 Result;
	Result.X = V.X * Scalar;
	Result.Y = V.Y * Scalar;
	Result.Z = V.Z * Scalar;
#endif
	return Result;
}

function inline v3
operator/(const v3 &V, f32 Scalar)
{
#if ENABLE_SIMD
	v3 Result = _mm_mul_ps(V, _mm_set1_ps(1.0f / Scalar));
#else
	v3 Result;
	f32 OneOverSalar = 1.0f / Scalar;
	Result.X = V.X * OneOverSalar;
	Result.Y = V.Y * OneOverSalar;
	Result.Z = V.Z * OneOverSalar;
#endif
	return Result;
}

function inline v3
operator/=(v3 &V, f32 Scalar)
{
#if ENABLE_SIMD
	V = _mm_mul_ps(V, _mm_set1_ps(1.0f / Scalar));
#else
	f32 OneOverSalar = 1.0f / Scalar;
	V.X *= OneOverSalar;
	V.Y *= OneOverSalar;
	V.Z *= OneOverSalar;
#endif
	return V;
}

function inline f32
SquareRoot(f32 Scalar)
{
	return sqrtf(Scalar);
}

function inline f32
Dot(const v3 &A, const v3 &B)
{
#if ENABLE_SIMD
	const i32 Mask = 0xff;
	v3 WideDot = _mm_dp_ps(A, B, Mask);
	f32 Result = VectorComponent(WideDot, 0);
#else
	f32 Result = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
#endif
	return Result;
}

function inline f32 Length(const v3 &V)
{
#if ENABLE_SIMD
	f32 Result = SquareRoot(Dot(V, V));
#else
	f32 Result = SquareRoot(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
#endif
	return Result;
}

function inline f32 Abs(f32 Scalar)
{
	return fabsf(Scalar);
}

function inline v3 Abs(const v3 &V)
{
#if ENABLE_SIMD
	// @todo(amer): not sure if this works so figure out how to do absloute values on floats
	v3 Result = _mm_cvtepi32_ps(_mm_abs_epi32(_mm_cvtps_epi32(V)));
#else
	v3 Result;
	Result.X = Abs(V.X);
	Result.Y = Abs(V.Y);
	Result.Z = Abs(V.Z);
#endif
	return Result;
}

function inline v3
Reflect(const v3& V, const v3& Normal)
{
	return V - 2.0f * Dot(V, Normal) * Normal;
}

// todo(harlequin): are the branchless versions actually faster ?
function inline f32
Minimum(f32 A, f32 B)
{
	f32 Selection = (f32)(A <= B);
	return A * Selection + (1.0f - Selection) * B;
}

function inline f32
Maximium(f32 A, f32 B)
{
	f32 Selection = (f32)(A >= B);
	return A * Selection + (1.0f - Selection) * B;
}

function inline f32
Clamp(f32 Value, f32 MinValue, f32 MaxValue)
{
	return Minimum(Maximium(Value, MinValue), MaxValue);
}

function inline v3
Clamp(const v3 &V,
      const v3 &MinValues,
	  const v3 &MaxValues)
{
#if ENABLE_SIMD
	v3 Result = _mm_min_ps(_mm_max_ps(V, MinValues), MaxValues);
#else
	v3 Result;

	Result.X = Clamp(V.X, MinValues.X, MaxValues.X);
	Result.Y = Clamp(V.Y, MinValues.Y, MaxValues.Y);
	Result.Z = Clamp(V.Z, MinValues.Z, MaxValues.Z);
#endif
	return Result;
}

function inline v3
Normalize(const v3 &V,
		  const v3 &ReturnedV3IfVLengthIsZero = { 0.0f, 0.0f, 0.0f });

function inline color8 NormalizedColorToColor8(const v3 &NormalizedColor)
{
#if ENABLE_SIMD
	color8 Result;
	v3 Color255 = _mm_mul_ps(NormalizedColor, _mm_set1_ps(255.0f));
	Result.R = (u8)VectorComponent(Color255, 0);
	Result.G = (u8)VectorComponent(Color255, 1);
	Result.B = (u8)VectorComponent(Color255, 2);
#else
	color8 Result;
	Result.R = (u8)(NormalizedColor.X * 255.0f);
	Result.G = (u8)(NormalizedColor.Y * 255.0f);
	Result.B = (u8)(NormalizedColor.Z * 255.0f);
#endif
	return Result;
}

function inline v3
Pow(const v3 &Base, f32 Exponent)
{
#if ENABLE_SIMD
	v3 Result = _mm_pow_ps(Base, _mm_set1_ps(Exponent));
#else
	v3 Result;
	Result.X = Pow(Base.X, Exponent);
	Result.Y = Pow(Base.Y, Exponent);
	Result.Z = Pow(Base.Z, Exponent);
#endif
return Result;
}

#if 0
v4 V4(const v3& V, f32 W = 0.0f)
{
#if ENABLE_SIMD
	v4 Result = _mm_set_ps();
#else
	v4 Result;
	Result.X = V.X;
	Result.Y = V.Y;
	Result.Z = V.Z;
	Result.W = W;
#endif
	return Result;
}
#endif

struct ray
{
	v3 Origin;
	v3 Direction;
};

function inline ray
RayOriginDirection(const v3 &Origin, const v3 &Direction)
{
	ray Result;
#if ENABLE_SIMD
	Result.Origin = Origin;
	Result.Direction = Direction;
#else
	Result.Origin = Origin;
	Result.Direction = Direction;
#endif
	return Result;
}

function inline v3
SampleRay(const ray &Ray, f32 T)
{
#if ENABLE_SIMD
	v3 Result = _mm_add_ps(Ray.Origin, _mm_mul_ps(Ray.Direction, V3(T)));
#else
	v3 Result = Ray.Origin + Ray.Direction * T;
#endif
	return Result;
}

struct sphere
{
	v3  Center;
	f32 Radius;
};

function inline sphere
SphereCenterRadius(const v3 &Center, f32 Radius)
{
	sphere Result;
	Result.Center = Center;
	Result.Radius = Radius;
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

function inline v3
NormalToNormalizedColor(const v3 &Normal)
{
	return (Normal + V3(1.0f)) * 0.5f;
}

function inline bool
RayCastSphere(const ray    &Ray,
			  const sphere &Sphere,
			  f32          *OutT)
{
	const v3 RayOriginToSphereCenter = Ray.Origin - Sphere.Center;
	// note(harlequin): should we cache Ray.Direction * Ray.Direction ?
	const f32 A = Dot(Ray.Direction, Ray.Direction);
	const f32 HalfB = Dot(RayOriginToSphereCenter, Ray.Direction);
	// note(harlequin): should we cache Sphere.Radius * Sphere.Radius ?
	const f32 C = Dot(RayOriginToSphereCenter, RayOriginToSphereCenter) - Sphere.Radius * Sphere.Radius;
	const f32 DiscriminantOver4 = HalfB * HalfB - A * C;
	const f32 ClosestT = (-HalfB - SquareRoot(DiscriminantOver4)) / A;
	*OutT = ClosestT;
	return DiscriminantOver4 > 0.0f && ClosestT > 0.0f;
}

#if 0
function inline bool
RayCastSphereSIMD(const ray    &Ray,
				  const sphere &Sphere,
				  f32          *OutT)
{

	const i32 Mask = 0xff;

	v4 _RayOrigin = V4(Ray.Origin);
	v4 _RayDirection = V4(Ray.Direction);
	v4 _SphereCenter = V4(Sphere.Center);

	__m128 RayOrigin = _mm_load_ps((const f32*)&_RayOrigin);
	__m128 RayDirection = _mm_load_ps((const f32*)&_RayDirection);
	__m128 SphereCenter = _mm_load_ps((const f32*)&_SphereCenter);

	// const v3 RayOriginToSphereCenter = Ray.Origin - Sphere.Center;
	__m128 RayOriginToSphereCenter = _mm_sub_ps(RayOrigin, SphereCenter);

	// const f32 A = Dot(Ray.Direction, Ray.Direction);
	__m128 RayDirectionSquared = _mm_dp_ps(RayDirection, RayDirection, Mask);
	const f32 A = ((f32*)&RayDirectionSquared)[0];

	// const f32 HalfB = Dot(RayOriginToSphereCenter, Ray.Direction);
	__m128 HalfBMul = _mm_dp_ps(RayOriginToSphereCenter, RayDirection, Mask);
	const f32 HalfB = ((f32*)&HalfBMul)[0];

	// note(harlequin): should we cache Sphere.Radius * Sphere.Radius ?
	// const f32 C = Dot(RayOriginToSphereCenter, RayOriginToSphereCenter) - Sphere.Radius * Sphere.Radius;
	__m128 RayOriginToSphereCenterSquared = _mm_dp_ps(RayOriginToSphereCenter, RayOriginToSphereCenter, Mask);
	const f32 C = ((f32*)&RayOriginToSphereCenterSquared)[0] - Sphere.Radius * Sphere.Radius;

	const f32 DiscriminantOver4 = HalfB * HalfB - A * C;
	const f32 ClosestT = (-HalfB - sqrtf(DiscriminantOver4)) / A;
	*OutT = ClosestT;
	return DiscriminantOver4 > 0.0f && ClosestT > 0.0f;
}
#endif