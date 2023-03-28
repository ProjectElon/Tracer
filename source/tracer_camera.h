#pragma once

#include "tracer_core.h"
#include "tracer_math.h"

struct camera
{
    f32 FocalLength;
    f32 AspectRatio;

    v3 ViewportSize;

    v3 Origin;
    v3 LowerLeftCornor;

    u32  RayCount;
    ray *Rays;
};

function void
ResizeCamera(camera *Camera,
             u32     Width,
             u32     Height);

function void
InitializeCamera(camera *Camera,
                 u32     FrameBufferWidth,
                 u32     FrameBufferHeight,
                 f32     FocalLength,
                 v3      Origin);