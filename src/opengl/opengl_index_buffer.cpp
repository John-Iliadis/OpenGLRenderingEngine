//
// Created by Gianni on 23/01/2025.
//

#include "opengl_index_buffer.hpp"

OpenGLIndexBuffer::OpenGLIndexBuffer()
    : mRendererID()
    , mCount()
{
}

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count, const void *data)
    : mCount(count)
{
    glCreateBuffers(1, &mRendererID);
    glNamedBufferData(mRendererID, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
    glDeleteBuffers(1, &mRendererID);
}

OpenGLIndexBuffer::OpenGLIndexBuffer(OpenGLIndexBuffer &&other) noexcept
{
    mRendererID = other.mRendererID;
    mCount = other.mCount;

    other.mRendererID = 0;
    other.mCount = 0;
}

OpenGLIndexBuffer &OpenGLIndexBuffer::operator=(OpenGLIndexBuffer &&other) noexcept
{
    if (this != &other)
    {
        glDeleteBuffers(1, &mRendererID);

        mRendererID = other.mRendererID;
        mCount = other.mCount;

        other.mRendererID = 0;
        other.mCount = 0;
    }

    return *this;
}

void OpenGLIndexBuffer::update(uint32_t offset, uint32_t size, const void *data)
{
    glNamedBufferSubData(mRendererID, offset, size, data);
}

void OpenGLIndexBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
}

void OpenGLIndexBuffer::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

uint32_t OpenGLIndexBuffer::id() const
{
    return mRendererID;
}

uint32_t OpenGLIndexBuffer::count() const
{
    return mCount;
}