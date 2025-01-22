//
// Created by Gianni on 23/01/2025.
//

#include "opengl_vertex_buffer.hpp"

OpenGLVertexBuffer::OpenGLVertexBuffer()
    : mRendererID()
{
}

OpenGLVertexBuffer::OpenGLVertexBuffer(GLenum usage, uint32_t size, const void* data)
    : mSize(size)
{
    glCreateBuffers(1, &mRendererID);
    glNamedBufferData(mRendererID, size, data, usage);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
    glDeleteBuffers(1, &mRendererID);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(OpenGLVertexBuffer &&other) noexcept
{
    mRendererID = other.mRendererID;
    mSize = other.mSize;

    other.mRendererID = 0;
    other.mSize = 0;
}

OpenGLVertexBuffer &OpenGLVertexBuffer::operator=(OpenGLVertexBuffer &&other) noexcept
{
    if (this != &other)
    {
        glDeleteBuffers(1, &mRendererID);

        mRendererID = other.mRendererID;
        mSize = other.mSize;

        other.mRendererID = 0;
        other.mSize = 0;
    }

    return *this;
}

void OpenGLVertexBuffer::update(uint32_t offset, uint32_t size, const void *data)
{
    glNamedBufferSubData(mRendererID, offset, size, data);
}

void OpenGLVertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
}

void OpenGLVertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

uint32_t OpenGLVertexBuffer::id() const
{
    return mRendererID;
}

uint32_t OpenGLVertexBuffer::size() const
{
    return mSize;
}
