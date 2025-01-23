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

class VertexBufferLayout
{
public:
    VertexBufferLayout();
    VertexBufferLayout(uint32_t stride, StepRate stepRate, std::initializer_list<VertexAttribute> attributes);

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

class VertexBuffer
{
public:
    VertexBuffer();
    VertexBuffer(GLenum usage, uint32_t size, const void* data);
    ~VertexBuffer();

    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    void update(uint32_t offset, uint32_t size, const void* data);

    void bind() const;
    void unbind() const;

    uint32_t id() const;
    uint32_t size() const;

private:
    uint32_t mRendererID;
    uint32_t mSize;
};

class IndexBuffer
{
public:
    IndexBuffer();
    IndexBuffer(uint32_t count, const void* data);
    ~IndexBuffer();

    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    void update(uint32_t offset, uint32_t size, const void *data);

    void bind() const;
    void unbind() const;

    uint32_t id() const;
    uint32_t count() const;

private:
    uint32_t mRendererID;
    uint32_t mCount;
};

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    void attachVertexBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout, uint32_t bindingIndex);
    void attachIndexBuffer(const IndexBuffer& indexBuffer);

    void bind() const;
    void unbind() const;

    uint32_t id() const;

private:
    uint32_t mRendererID;
};

class ShaderBuffer
{
public:
    ShaderBuffer();
    ShaderBuffer(GLenum type, GLenum usage, uint32_t binding, uint32_t size, const void* data);
    ~ShaderBuffer();

    ShaderBuffer(ShaderBuffer&& other) noexcept;
    ShaderBuffer& operator=(ShaderBuffer&& other) noexcept;

    ShaderBuffer(const ShaderBuffer&) = delete;
    ShaderBuffer& operator=(const ShaderBuffer&) = delete;

    void update(uint32_t offset, uint32_t size, const void *data);

    void bind() const;
    void unbind() const;

    uint32_t id() const;
    uint32_t size() const;
    uint32_t binding() const;

private:
    GLenum mType;
    uint32_t mRendererID;
    int32_t mBinding;
    uint32_t mSize;
};

#endif //OPENGLRENDERINGENGINE_OPENGL_BUFFER_HPP
