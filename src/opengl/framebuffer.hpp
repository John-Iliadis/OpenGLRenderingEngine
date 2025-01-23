//
// Created by Gianni on 22/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_FRAMEBUFFER_HPP
#define OPENGLRENDERINGENGINE_FRAMEBUFFER_HPP

#include <glad/glad.h>
#include "texture.hpp"

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    void bind() const;
    void unbind() const;

    void addColorAttachment(const Texture& texture, uint32_t index);
    void addDepthAttachment(const Texture& texture);
    void addDepthStencilAttachment(const Texture& texture);

    void setDrawBuffers(std::initializer_list<uint32_t> drawBufferIndices);
    void setDepthStencilOnly(bool depthStencilOnly);

    uint32_t id() const;

private:
    uint32_t mRendererID;
};

#endif //OPENGLRENDERINGENGINE_FRAMEBUFFER_HPP
