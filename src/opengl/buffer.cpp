//
// Created by Gianni on 23/01/2025.
//

#include "buffer.hpp"

// -- VertexAttribute -- //

VertexAttribute::VertexAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset)
    : index(index)
    , count(count)
    , type(type)
    , offset(offset)
{
}

// -- VertexBufferLayout -- //

VertexBufferLayout::VertexBufferLayout()
    : mStride()
    , mStepRate(StepRate::Vertex)
{
}

VertexBufferLayout::VertexBufferLayout(uint32_t stride, StepRate stepRate, std::initializer_list<VertexAttribute> attributes)
    : mStride(stride)
    , mStepRate(stepRate)
    , mAttributes(attributes)
{
}

void VertexBufferLayout::setStride(uint32_t stride)
{
    mStride = stride;
}

void VertexBufferLayout::setStepRate(StepRate stepRate)
{
    mStepRate = stepRate;
}

void VertexBufferLayout::addAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset)
{
    mAttributes.emplace_back(index, count, type, offset);
}

uint32_t VertexBufferLayout::stride() const
{
    return mStride;
}

uint32_t VertexBufferLayout::stepRate() const
{
    return static_cast<uint32_t>(mStepRate);
}

const std::vector<VertexAttribute> &VertexBufferLayout::attributes() const
{
    return mAttributes;
}

// -- VertexBuffer -- //

VertexBuffer::VertexBuffer()
    : mRendererID()
{
}

VertexBuffer::VertexBuffer(GLenum usage, uint32_t size, const void* data)
    : mSize(size)
{
    glCreateBuffers(1, &mRendererID);
    glNamedBufferData(mRendererID, size, data, usage);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &mRendererID);
}

VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept
{
    mRendererID = other.mRendererID;
    mSize = other.mSize;

    other.mRendererID = 0;
    other.mSize = 0;
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) noexcept
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

void VertexBuffer::update(uint32_t offset, uint32_t size, const void *data)
{
    glNamedBufferSubData(mRendererID, offset, size, data);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

uint32_t VertexBuffer::id() const
{
    return mRendererID;
}

uint32_t VertexBuffer::size() const
{
    return mSize;
}

// -- IndexBuffer -- //

IndexBuffer::IndexBuffer()
    : mRendererID()
    , mCount()
{
}

IndexBuffer::IndexBuffer(uint32_t count, const void *data)
    : mCount(count)
{
    glCreateBuffers(1, &mRendererID);
    glNamedBufferData(mRendererID, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &mRendererID);
}

IndexBuffer::IndexBuffer(IndexBuffer &&other) noexcept
{
    mRendererID = other.mRendererID;
    mCount = other.mCount;

    other.mRendererID = 0;
    other.mCount = 0;
}

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other) noexcept
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

void IndexBuffer::update(uint32_t offset, uint32_t size, const void *data)
{
    glNamedBufferSubData(mRendererID, offset, size, data);
}

void IndexBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
}

void IndexBuffer::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

uint32_t IndexBuffer::id() const
{
    return mRendererID;
}

uint32_t IndexBuffer::count() const
{
    return mCount;
}

// -- VertexArray -- //

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &mRendererID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &mRendererID);
}

VertexArray::VertexArray(VertexArray &&other) noexcept
{
    mRendererID = other.mRendererID;
    other.mRendererID = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&other) noexcept
{
    if (this != &other)
    {
        glDeleteVertexArrays(1, &mRendererID);

        mRendererID = other.mRendererID;
        other.mRendererID = 0;
    }

    return *this;
}

// todo: check if this is right. May still need to set divisor for each attrib
void VertexArray::attachVertexBuffer(const VertexBuffer &vertexBuffer, const VertexBufferLayout &layout, uint32_t bindingIndex)
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

void VertexArray::attachIndexBuffer(const IndexBuffer &indexBuffer)
{
    glVertexArrayElementBuffer(mRendererID, indexBuffer.id());
}

void VertexArray::bind() const
{
    glBindVertexArray(mRendererID);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

uint32_t VertexArray::id() const
{
    return mRendererID;
}

// -- ShaderBuffer -- //

ShaderBuffer::ShaderBuffer()
    : mType(GL_INVALID_ENUM)
    , mRendererID()
    , mBinding(-1)
    , mSize()
{
}

ShaderBuffer::ShaderBuffer(GLenum type, GLenum usage, uint32_t binding, uint32_t size, const void* data)
    : mType(type)
    , mBinding(binding)
    , mSize(size)
{
    glCreateBuffers(1, &mRendererID);
    glBindBufferBase(type, binding, mRendererID);
    glNamedBufferData(mRendererID, size, data, usage);
}

ShaderBuffer::~ShaderBuffer()
{
    glDeleteBuffers(1, &mRendererID);
}

ShaderBuffer::ShaderBuffer(ShaderBuffer &&other) noexcept
{
    mType = other.mType;
    mRendererID = other.mRendererID;
    mBinding = other.mBinding;
    mSize = other.mSize;

    other.mType = GL_INVALID_ENUM;
    other.mRendererID = 0;
    other.mBinding = -1;
    other.mSize = 0;
}

ShaderBuffer &ShaderBuffer::operator=(ShaderBuffer &&other) noexcept
{
    if (this != &other)
    {
        glDeleteBuffers(1, &mRendererID);

        mType = other.mType;
        mRendererID = other.mRendererID;
        mBinding = other.mBinding;
        mSize = other.mSize;

        other.mType = GL_INVALID_ENUM;
        other.mRendererID = 0;
        other.mBinding = -1;
        other.mSize = 0;
    }

    return *this;
}

void ShaderBuffer::update(uint32_t offset, uint32_t size, const void *data)
{
    glNamedBufferSubData(mRendererID, offset, size, data);
}

void ShaderBuffer::bind() const
{
    glBindBuffer(mType, mRendererID);
}

void ShaderBuffer::unbind() const
{
    glBindBuffer(mType, 0);
}

uint32_t ShaderBuffer::id() const
{
    return mRendererID;
}

uint32_t ShaderBuffer::binding() const
{
    return mBinding;
}

uint32_t ShaderBuffer::size() const
{
    return mSize;
}
