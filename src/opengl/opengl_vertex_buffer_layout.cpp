//
// Created by Gianni on 22/01/2025.
//

#include "opengl_vertex_buffer_layout.hpp"

OpenGLVertexBufferLayout::OpenGLVertexBufferLayout()
    : mStride()
{
}

OpenGLVertexBufferLayout::OpenGLVertexBufferLayout(uint32_t stride, std::initializer_list<VertexAttribute> attributes)
    : mStride(stride)
    , mAttributes(attributes)
{
}

void OpenGLVertexBufferLayout::setStride(uint32_t stride)
{
    mStride = stride;
}

void OpenGLVertexBufferLayout::addAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset)
{
    mAttributes.emplace_back(index, count, type, offset);
}

uint32_t OpenGLVertexBufferLayout::stride() const
{
    return mStride;
}

const std::vector<VertexAttribute> &OpenGLVertexBufferLayout::attributes() const
{
    return mAttributes;
}
