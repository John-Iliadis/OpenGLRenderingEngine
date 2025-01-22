//
// Created by Gianni on 22/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_TEXTURE2D_HPP
#define OPENGLRENDERINGENGINE_OPENGL_TEXTURE2D_HPP

#include <glad/glad.h>
#include "opengl_texture.hpp"

class OpenGLTexture2D
{
public:
    OpenGLTexture2D();
    OpenGLTexture2D(const TextureSpecification& spec);
    OpenGLTexture2D(const TextureSpecification& spec, const void* textureData);
    OpenGLTexture2D(const TextureSpecification& spec, const std::string& texturePath);
    ~OpenGLTexture2D();

    OpenGLTexture2D(const OpenGLTexture2D&) = delete;
    OpenGLTexture2D& operator=(const OpenGLTexture2D&) = delete;

    OpenGLTexture2D(OpenGLTexture2D&& other) noexcept;
    OpenGLTexture2D& operator=(OpenGLTexture2D&& other) noexcept;

    void resize(int32_t width, int32_t height);

    void bind(uint32_t unit);
    void unbind(uint32_t unit);

    uint32_t id() const;
    int32_t width() const;
    int32_t height() const;

private:
    void create();
    void destroy();
    void setTextureParameters(TextureWrap wrapMode, TextureFilter filterMode);
    void uploadTextureData(const void* textureData);

private:
    uint32_t mRendererID;
    TextureSpecification mSpecification;
};


#endif //OPENGLRENDERINGENGINE_OPENGL_TEXTURE2D_HPP
