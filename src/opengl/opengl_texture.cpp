//
// Created by Gianni on 22/01/2025.
//

#include "opengl_texture.hpp"
#include <stb/stb_image.h>
#include "../utils.hpp"

GLenum toGLenum(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::R8: return GL_R8;
        case TextureFormat::RGB8: return GL_RGB8;
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::RGB32F: return GL_RGB32F;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        case TextureFormat::D32: return GL_DEPTH_COMPONENT32;
        case TextureFormat::D24S8: return GL_DEPTH24_STENCIL8;
        default: assert(false);
    }
}

GLenum toGLenum(TextureDataType type)
{
    switch (type)
    {
        case TextureDataType::UINT8: return GL_UNSIGNED_BYTE;
        case TextureDataType::FLOAT: return GL_FLOAT;
        default: assert(false);
    }
}

GLenum toGLenum(TextureWrap wrapMode)
{
    switch (wrapMode)
    {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: assert(false);
    }
}

GLenum toGLenumMinFilter(TextureFilter filterMode)
{
    switch (filterMode)
    {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Bilinear: return GL_LINEAR;
        case TextureFilter::Trilinear: GL_LINEAR_MIPMAP_LINEAR;
        case TextureFilter::Anisotropic: return GL_LINEAR_MIPMAP_LINEAR;
        default: assert(false);
    }
}

GLenum toGLenumMagFilter(TextureFilter filterMode)
{
    switch (filterMode)
    {
        case TextureFilter::Nearest: GL_NEAREST;
        case TextureFilter::Bilinear: GL_LINEAR;
        case TextureFilter::Trilinear:  GL_LINEAR;
        case TextureFilter::Anisotropic: GL_LINEAR;
        default: assert(false);
    }
}

uint32_t calculateMipLevels(int32_t textureWidth, int32_t textureHeight)
{
    return glm::floor(glm::log2((float)glm::max(textureWidth, textureHeight))) + 1;
}

std::shared_ptr<uint8_t> loadImage(const std::string& imagePath, int32_t* width, int32_t* height, int requiredChannels)
{
    uint8_t* data = stbi_load(imagePath.c_str(), width, height, nullptr, requiredChannels);
    check(data, std::format("Failed to load {}", imagePath).c_str());

    return std::shared_ptr<uint8_t>(data, [] (uint8_t* d) { stbi_image_free(d); });
}

std::shared_ptr<float> loadImageHDR(const std::string& imagePath, int32_t* width, int32_t* height, int requiredChannels)
{
    float* data = stbi_loadf(imagePath.c_str(), width, height, nullptr, requiredChannels);
    check(data, std::format("Failed to load {}", imagePath).c_str());

    return std::shared_ptr<float>(data, [] (float* d) { stbi_image_free(d); });
}

int getRequiredChannels(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::RGB8:
        case TextureFormat::RGB32F: return 3;
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA32F: return 4;
        default: assert(false);
    }
}
