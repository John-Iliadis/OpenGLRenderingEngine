//
// Created by Gianni on 22/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_FRAMEBUFFER_HPP
#define OPENGLRENDERINGENGINE_OPENGL_FRAMEBUFFER_HPP

#include <glad/glad.h>
#include "opengl_texture.hpp"

class OpenGLFramebuffer
{
public:
    OpenGLFramebuffer();
    ~OpenGLFramebuffer();

    OpenGLFramebuffer(OpenGLFramebuffer&& other) noexcept;
    OpenGLFramebuffer& operator=(OpenGLFramebuffer&& other) noexcept;

    OpenGLFramebuffer(const OpenGLFramebuffer&) = delete;
    OpenGLFramebuffer& operator=(const OpenGLFramebuffer&) = delete;

    void bind() const;
    void unbind() const;

    void addColorAttachment(const OpenGLTexture& texture, uint32_t index);
    void addDepthAttachment(const OpenGLTexture& texture);
    void addDepthStencilAttachment(const OpenGLTexture2D& texture);

    void setDrawBuffers(const std::initializer_list<uint32_t> drawBufferIndices);
    void setDepthStencilOnly(bool depthStencilOnly);

    uint32_t id() const;

private:
    uint32_t mRendererID;
};

#endif //OPENGLRENDERINGENGINE_OPENGL_FRAMEBUFFER_HPP
