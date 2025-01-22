//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_INDEX_BUFFER_HPP
#define OPENGLRENDERINGENGINE_OPENGL_INDEX_BUFFER_HPP

#include <glad/glad.h>

class OpenGLIndexBuffer
{
public:
    OpenGLIndexBuffer();
    OpenGLIndexBuffer(uint32_t count, const void* data);
    ~OpenGLIndexBuffer();

    OpenGLIndexBuffer(OpenGLIndexBuffer&& other) noexcept;
    OpenGLIndexBuffer& operator=(OpenGLIndexBuffer&& other) noexcept;

    OpenGLIndexBuffer(const OpenGLIndexBuffer&) = delete;
    OpenGLIndexBuffer& operator=(const OpenGLIndexBuffer&) = delete;

    void update(uint32_t offset, uint32_t size, const void *data);

    void bind() const;
    void unbind() const;

    uint32_t id() const;
    uint32_t count() const;

private:
    uint32_t mRendererID;
    uint32_t mCount;
};


#endif //OPENGLRENDERINGENGINE_OPENGL_INDEX_BUFFER_HPP
