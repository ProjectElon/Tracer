#pragma once

#include "tracer_core.h"
#include "tracer_math.h"

struct frame_buffer
{
    u32  Width;
    u32  Height;
    v3  *Pixels;
};

function void
InitializeFrameBuffer(frame_buffer *FrameBuffer,
                      u32           Width,
                      u32           Height);

function void
ResizeFrameBuffer(frame_buffer *FrameBuffer,
                  u32           NewWidth,
                  u32           NewHeight);

function void
ClearFrameBuffer(frame_buffer *FrameBuffer);