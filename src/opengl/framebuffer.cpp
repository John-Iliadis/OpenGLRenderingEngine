//
// Created by Gianni on 22/01/2025.
//

#include "framebuffer.hpp"

Framebuffer::Framebuffer()
{
    glCreateFramebuffers(1, &mRendererID);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &mRendererID);
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept
{
    mRendererID = other.mRendererID;
    other.mRendererID = 0;
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other) noexcept
{
    if (this != &other)
    {
        glDeleteFramebuffers(1, &mRendererID);

        mRendererID = other.mRendererID;
        other.mRendererID = 0;
    }

    return *this;
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
}

void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t Framebuffer::id() const
{
    return mRendererID;
}

void Framebuffer::addColorAttachment(const Texture &texture, uint32_t index)
{
    glNamedFramebufferTexture(mRendererID, GL_COLOR_ATTACHMENT0 + index, texture.id(), 0);
}

void Framebuffer::addDepthAttachment(const Texture &texture)
{
    glNamedFramebufferTexture(mRendererID, GL_DEPTH_ATTACHMENT, texture.id(), 0);
}

void Framebuffer::addDepthStencilAttachment(const Texture &texture)
{
    glNamedFramebufferTexture(mRendererID, GL_DEPTH_STENCIL_ATTACHMENT, texture.id(), 0);
}

void Framebuffer::setDrawBuffers(std::initializer_list<uint32_t> drawBufferIndices)
{
    bind();

    std::vector<GLenum> drawBuffers;
    for (uint32_t i : drawBufferIndices)
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);

    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    unbind();
}

void Framebuffer::setDepthStencilOnly(bool depthStencilOnly)
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
