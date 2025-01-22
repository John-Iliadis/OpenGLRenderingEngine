//
// Created by Gianni on 23/01/2025.
//

#include "opengl_buffer.hpp"

// -- VertexAttribute -- //

VertexAttribute::VertexAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset)
    : index(index)
    , count(count)
    , type(type)
    , offset(offset)
{
}

// -- OpenGLVertexBufferLayout -- //

OpenGLVertexBufferLayout::OpenGLVertexBufferLayout()
    : mStride()
    , mStepRate(StepRate::Vertex)
{
}

OpenGLVertexBufferLayout::OpenGLVertexBufferLayout(uint32_t stride, StepRate stepRate, std::initializer_list<VertexAttribute> attributes)
    : mStride(stride)
    , mStepRate(stepRate)
    , mAttributes(attributes)
{
}

void OpenGLVertexBufferLayout::setStride(uint32_t stride)
{
    mStride = stride;
}

void OpenGLVertexBufferLayout::setStepRate(StepRate stepRate)
{
    mStepRate = stepRate;
}

void OpenGLVertexBufferLayout::addAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset)
{
    mAttributes.emplace_back(index, count, type, offset);
}

uint32_t OpenGLVertexBufferLayout::stride() const
{
    return mStride;
}

uint32_t OpenGLVertexBufferLayout::stepRate() const
{
    return static_cast<uint32_t>(mStepRate);
}

const std::vector<VertexAttribute> &OpenGLVertexBufferLayout::attributes() const
{
    return mAttributes;
}

// -- OpenGLVertexBuffer -- //

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

// -- OpenGLIndexBuffer -- //

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

// -- OpenGLVertexArray -- //

OpenGLVertexArray::OpenGLVertexArray()
{
    glCreateVertexArrays(1, &mRendererID);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
    glDeleteVertexArrays(1, &mRendererID);
}

OpenGLVertexArray::OpenGLVertexArray(OpenGLVertexArray &&other) noexcept
{
    mRendererID = other.mRendererID;
    other.mRendererID = 0;
}

OpenGLVertexArray &OpenGLVertexArray::operator=(OpenGLVertexArray &&other) noexcept
{
    if (this != &other)
    {
        glDeleteVertexArrays(1, &mRendererID);

        mRendererID = other.mRendererID;
        other.mRendererID = 0;
    }

    return *this;
}

// todo: check if this is right
void OpenGLVertexArray::attachVertexBuffer(const OpenGLVertexBuffer &vertexBuffer, const OpenGLVertexBufferLayout &layout, uint32_t bindingIndex)
{
    glVertexArrayVertexBuffer(mRendererID, bindingIndex, vertexBuffer.id(), 0, layout.stride());
    glVertexArrayBindingDivisor(mRendererID, bindingIndex, layout.stepRate());

    for (const auto& vertexAttribute : layout.attributes())
    {
        glEnableVertexArrayAttrib(mRendererID, vertexAttribute.index);
        glVertexArrayAttribBinding(mRendererID, vertexAttribute.index, bindingIndex);
        glVertexArrayAttribFormat(mRendererID,
                                  vertexAttribute.index,
                                  vertexAttribute.count,
                                  vertexAttribute.type,
                                  GL_FALSE,
                                  vertexAttribute.offset);
    }
}

void OpenGLVertexArray::attachIndexBuffer(const OpenGLIndexBuffer &indexBuffer)
{
    glVertexArrayElementBuffer(mRendererID, indexBuffer.id());
}

void OpenGLVertexArray::bind() const
{
    glBindVertexArray(mRendererID);
}

void OpenGLVertexArray::unbind() const
{
    glBindVertexArray(0);
}

uint32_t OpenGLVertexArray::id() const
{
    return mRendererID;
}
