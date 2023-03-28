#pragma once

#include "tracer_core.h"
#include "tracer_framebuffer.h"

struct opengl_texture
{
    u32 Handle;
    u32 Width;
    u32 Height;
    u32 Format;
    u32 InternalFormat;
    u32 PixelType;
};

function bool
InitializeOpenglTexture(opengl_texture *Texture,
                        u32             Width,
                        u32             Height,
                        u32             Format,
                        u32             InternalFormat,
                        u32             PixelType);

function void
ResizeTexture(opengl_texture *Texture,
              u32             NewWidth,
              u32             NewHeight);

function void
CopyFrameBufferToTexture(frame_buffer   *FrameBuffer,
                         opengl_texture *Texture);