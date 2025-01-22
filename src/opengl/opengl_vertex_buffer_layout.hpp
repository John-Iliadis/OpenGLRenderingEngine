//
// Created by Gianni on 22/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_VERTEX_BUFFER_LAYOUT_HPP
#define OPENGLRENDERINGENGINE_OPENGL_VERTEX_BUFFER_LAYOUT_HPP

#include <glad/glad.h>

struct VertexAttribute;

class OpenGLVertexBufferLayout
{
public:
    OpenGLVertexBufferLayout();
    OpenGLVertexBufferLayout(uint32_t stride, std::initializer_list<VertexAttribute> attributes);

    void setStride(uint32_t stride);
    void addAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset);

    uint32_t stride() const;
    const std::vector<VertexAttribute>& attributes() const;

private:
    uint32_t mStride;
    std::vector<VertexAttribute> mAttributes;
};

struct VertexAttribute
{
    uint32_t index;
    uint32_t count;
    GLenum type;
    uint32_t offset;

    VertexAttribute(uint32_t index, uint32_t count, GLenum type, uint32_t offset)
        : index(index)
        , count(count)
        , type(type)
        , offset(offset)
    {
    }
};

#endif //OPENGLRENDERINGENGINE_OPENGL_VERTEX_BUFFER_LAYOUT_HPP
