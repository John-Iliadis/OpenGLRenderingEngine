//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_BUFFER_HPP
#define OPENGLRENDERINGENGINE_OPENGL_BUFFER_HPP

#include <glad/glad.h>

struct VertexAttribute
{
    uint32_t index;
    uint32_t count;
    GLenum type;
    uint32_t offset;

    VertexAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset);
};

enum class StepRate : uint32_t
{
    Vertex = 0,
    Instance = 1
};

class OpenGLVertexBufferLayout
{
public:
    OpenGLVertexBufferLayout();
    OpenGLVertexBufferLayout(uint32_t stride, StepRate stepRate, std::initializer_list<VertexAttribute> attributes);

    void setStride(uint32_t stride);
    void setStepRate(StepRate stepRate);
    void addAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset);

    uint32_t stride() const;
    uint32_t stepRate() const;
    const std::vector<VertexAttribute>& attributes() const;

private:
    uint32_t mStride;
    StepRate mStepRate;
    std::vector<VertexAttribute> mAttributes;
};

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

class OpenGLVertexArray
{
public:
    OpenGLVertexArray();
    ~OpenGLVertexArray();

    OpenGLVertexArray(OpenGLVertexArray&& other) noexcept;
    OpenGLVertexArray& operator=(OpenGLVertexArray&& other) noexcept;

    OpenGLVertexArray(const OpenGLVertexArray&) = delete;
    OpenGLVertexArray& operator=(const OpenGLVertexArray&) = delete;

    void attachVertexBuffer(const OpenGLVertexBuffer& vertexBuffer, const OpenGLVertexBufferLayout& layout, uint32_t bindingIndex);
    void attachIndexBuffer(const OpenGLIndexBuffer& indexBuffer);

    void bind() const;
    void unbind() const;

    uint32_t id() const;

private:
    uint32_t mRendererID;
};

#endif //OPENGLRENDERINGENGINE_OPENGL_BUFFER_HPP
