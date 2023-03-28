#include "tracer_texture.h"

#include <glad/glad.h>

bool
InitializeOpenglTexture(opengl_texture *Texture,
                        u32             Width,
                        u32             Height,
                        u32             Format,
                        u32             InternalFormat,
                        u32             PixelType)
{
    Assert(Texture);
    Texture->Handle         = 0;
    Texture->Width          = Width;
    Texture->Height         = Height;
    Texture->Format         = Format;
    Texture->InternalFormat = InternalFormat;
    Texture->PixelType      = PixelType;

    glGenTextures(1, &Texture->Handle);
    Assert(Texture->Handle);
    glBindTexture(GL_TEXTURE_2D, Texture->Handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, PixelType, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void
ResizeTexture(opengl_texture *Texture,
              u32             NewWidth,
              u32             NewHeight)
{
    Texture->Width  = NewWidth;
    Texture->Height = NewHeight;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture->Handle);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 Texture->InternalFormat,
                 NewWidth,
                 NewHeight,
                 0,
                 Texture->Format,
                 Texture->PixelType,
                 0);
}

void
CopyFrameBufferToTexture(frame_buffer   *FrameBuffer,
                         opengl_texture *Texture)
{
    Assert(FrameBuffer->Width == Texture->Width);
    Assert(FrameBuffer->Height == Texture->Height);

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    Texture->Width,
                    Texture->Height,
                    Texture->Format,
                    Texture->PixelType,
                    FrameBuffer->Pixels);
}