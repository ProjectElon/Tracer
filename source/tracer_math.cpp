#include "tracer_math.h"

v3 Normalize(const v3& V, const v3 &ReturnedV3IfVLengthIsZero /* = { 0.0f, 0.0f, 0.0f }*/)
{
    f32 Magnitude = Length(V);

    if (Magnitude > 0.0f)
    {
        return V / Magnitude;
    }

    return ReturnedV3IfVLengthIsZero;
}

intersection_info
GetRaySphereIntersectionInfo(const ray    &Ray,
                             const sphere &Sphere,
                             f32           T)
{
    intersection_info Result;
    v3 Point  = SampleRay(Ray, T);
    v3 Normal = Normalize((Point - Sphere.Center));

    if (Dot(Ray.Direction, Normal) > 0.0f)
    {
        Result.Normal    = -Normal;
        Result.FrontFace = false;
    }
    else
    {
        Result.Normal    = Normal;
        Result.FrontFace = true;
    }
    Result.Point = Point;
    return Result;
}