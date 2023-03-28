#include "tracer_framebuffer.h"

void
InitializeFrameBuffer(frame_buffer *FrameBuffer,
                      u32           Width,
                      u32           Height)
{
    Assert(Width);
    Assert(Height);
    FrameBuffer->Width  = Width;
    FrameBuffer->Height = Height;
    FrameBuffer->Pixels = (v3*)malloc(sizeof(v3) * Width * Height);
}

void
ResizeFrameBuffer(frame_buffer *FrameBuffer,
                  u32           NewWidth,
                  u32           NewHeight)
{
    FrameBuffer->Width  = NewWidth;
    FrameBuffer->Height = NewHeight;
    FrameBuffer->Pixels = (v3*)realloc(FrameBuffer->Pixels, sizeof(v3) * NewWidth * NewHeight);
}

void
ClearFrameBuffer(frame_buffer *FrameBuffer)
{
    memset(FrameBuffer->Pixels, 0, sizeof(v3) * FrameBuffer->Width * FrameBuffer->Height);
}