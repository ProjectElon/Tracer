#include "tracer_camera.h"

function void
ResizeCamera(camera *Camera,
             u32     Width,
             u32     Height)
{
    f32 AspectRatio         = (f32)Width / (f32)Height;
    Camera->AspectRatio     = AspectRatio;
    f32 ViewportHeight      = 2.0f;
    Camera->ViewportSize    = V2(AspectRatio * ViewportHeight, ViewportHeight);
    v2 HalfViewportSize     = Camera->ViewportSize * 0.5f;
    Camera->LowerLeftCornor = Camera->Origin - V3(HalfViewportSize, 0.0f) - V3(0.0f, 0.0f, Camera->FocalLength);

    u32 RayCount = Width * Height;
    Camera->RayCount = RayCount;
    Camera->Rays     = (ray*)_aligned_realloc(Camera->Rays, sizeof(ray) * RayCount, alignof(ray));

    const u32 OneMinusWidth  = Width  - 1;
    const u32 OneMinusHeight = Height - 1;
    const f32 OneOverOneMinusWidth  = 1.0f / OneMinusWidth;
    const f32 OneOverOneMinusHeight = 1.0f / OneMinusHeight;

    for (u32 Y = 0; Y < Height; Y++)
    {
        for (u32 X = 0; X < Width; X++)
        {
            f32 SampleU = ((f32)X + 0.5f) * OneOverOneMinusWidth;
            f32 SampleV = ((f32)(Height - Y) - 0.5f) * OneOverOneMinusHeight;
            v3  SampleP = V3(SampleU * Camera->ViewportSize.X,
                             SampleV * Camera->ViewportSize.Y,
                             0.0f);

            v3  Direction  = Normalize(Camera->LowerLeftCornor + SampleP - Camera->Origin);
            u32 PixelIndex = GetPixelIndex(X, Y, Width);
            Camera->Rays[PixelIndex] = RayOriginDirection(Camera->Origin, Direction);
        }
    }
}

void
InitializeCamera(camera *Camera,
                 u32     FrameBufferWidth,
                 u32     FrameBufferHeight,
                 f32     FocalLength,
                 v3      Origin)
{
    Camera->FocalLength     = FocalLength;
    Camera->Origin          = Origin;
    ResizeCamera(Camera, FrameBufferWidth, FrameBufferHeight);
}