//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_VERTEX_BUFFER_HPP
#define OPENGLRENDERINGENGINE_OPENGL_VERTEX_BUFFER_HPP

#include <glad/glad.h>

class OpenGLVertexBuffer
{
public:
    OpenGLVertexBuffer();
    OpenGLVertexBuffer(GLenum usage, uint32_t size, const void* data);
    ~OpenGLVertexBuffer();

    OpenGLVertexBuffer(OpenGLVertexBuffer&& other) noexcept;
    OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&& other) noexcept;

    OpenGLVertexBuffer(const OpenGLVertexBuffer&) = delete;
    OpenGLVertexBuffer& operator=(const OpenGLVertexBuffer&) = delete;

    void update(uint32_t offset, uint32_t size, const void* data);

    void bind() const;
    void unbind() const;

    uint32_t id() const;
    uint32_t size() const;

private:
    uint32_t mRendererID;
    uint32_t mSize;
};

#endif //OPENGLRENDERINGENGINE_OPENGL_VERTEX_BUFFER_HPP
