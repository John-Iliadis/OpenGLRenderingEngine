//
// Created by Gianni on 22/01/2025.
//

#include "opengl_framebuffer.hpp"

OpenGLFramebuffer::OpenGLFramebuffer()
{
    glCreateFramebuffers(1, &mRendererID);
}

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    glDeleteFramebuffers(1, &mRendererID);
}

OpenGLFramebuffer::OpenGLFramebuffer(OpenGLFramebuffer &&other) noexcept
{
    mRendererID = other.mRendererID;
    other.mRendererID = 0;
}

OpenGLFramebuffer &OpenGLFramebuffer::operator=(OpenGLFramebuffer &&other) noexcept
{
    if (this != &other)
    {
        glDeleteFramebuffers(1, &mRendererID);

        mRendererID = other.mRendererID;
        other.mRendererID = 0;
    }

    return *this;
}

void OpenGLFramebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
}

void OpenGLFramebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t OpenGLFramebuffer::id() const
{
    return mRendererID;
}

void OpenGLFramebuffer::addColorAttachment(const OpenGLTexture &texture, uint32_t index)
{
    glNamedFramebufferTexture(mRendererID, GL_COLOR_ATTACHMENT0 + index, texture.id(), 0);
}

void OpenGLFramebuffer::addDepthAttachment(const OpenGLTexture &texture)
{
    glNamedFramebufferTexture(mRendererID, GL_DEPTH_ATTACHMENT, texture.id(), 0);
}

void OpenGLFramebuffer::addDepthStencilAttachment(const OpenGLTexture2D &texture)
{
    glNamedFramebufferTexture(mRendererID, GL_DEPTH_STENCIL_ATTACHMENT, texture.id(), 0);
}

void OpenGLFramebuffer::setDrawBuffers(const std::initializer_list<uint32_t> drawBufferIndices)
{
    bind();

    std::vector<GLenum> drawBuffers;
    for (uint32_t i : drawBufferIndices)
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);

    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    unbind();
}

void OpenGLFramebuffer::setDepthStencilOnly(bool depthStencilOnly)
{
    bind();

    if (depthStencilOnly)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    else
    {
        glDrawBuffer(GL_BACK);
        glDrawBuffer(GL_BACK);
    }

    unbind();
}
