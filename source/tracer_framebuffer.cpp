#include "tracer_framebuffer.h"
#include <stdlib.h>
void
InitializeFrameBuffer(frame_buffer *FrameBuffer,
                      u32           Width,
                      u32           Height)
{
    Assert(Width);
    Assert(Height);
    FrameBuffer->Width  = Width;
    FrameBuffer->Height = Height;
    FrameBuffer->Pixels = (v3*)_aligned_malloc(sizeof(v3) * Width * Height, alignof(v3));
}

void
ResizeFrameBuffer(frame_buffer *FrameBuffer,
                  u32           NewWidth,
                  u32           NewHeight)
{
    FrameBuffer->Width  = NewWidth;
    FrameBuffer->Height = NewHeight;
    FrameBuffer->Pixels = (v3*)_aligned_realloc(FrameBuffer->Pixels, sizeof(v3) * NewWidth * NewHeight, alignof(v3));
}

void
ClearFrameBuffer(frame_buffer *FrameBuffer)
{
    memset(FrameBuffer->Pixels, 0, sizeof(v3) * FrameBuffer->Width * FrameBuffer->Height);
}