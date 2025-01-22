//
// Created by Gianni on 22/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_TEXTURE_HPP
#define OPENGLRENDERINGENGINE_OPENGL_TEXTURE_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class TextureFormat
{
    R8,
    RGB8,
    RGBA8,
    RGB32F,
    RGBA32F,
    D32,
    D24S8
};

enum class TextureDataType
{
    UINT8,
    FLOAT
};

enum class TextureWrap
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

enum class TextureFilter
{
    Nearest,
    Bilinear,
    Trilinear,
    Anisotropic
};

struct TextureSpecification
{
    int32_t width;
    int32_t height;
    TextureFormat format;
    TextureDataType dataType;
    TextureWrap wrapMode;
    TextureFilter filterMode;
    bool generateMipMaps;
};

GLenum toGLenum(TextureFormat format);
GLenum toGLenum(TextureDataType type);
GLenum toGLenum(TextureWrap wrapMode);
GLenum toGLenumMinFilter(TextureFilter filterMode);
GLenum toGLenumMagFilter(TextureFilter filterMode);
uint32_t calculateMipLevels(int32_t textureWidth, int32_t textureHeight);

std::shared_ptr<uint8_t> loadImage(const std::string& imagePath, int32_t* width, int32_t* height, int requiredChannels);
std::shared_ptr<float> loadImageHDR(const std::string& imagePath, int32_t* width, int32_t* height, int requiredChannels);

int getRequiredChannels(TextureFormat format);

#endif //OPENGLRENDERINGENGINE_OPENGL_TEXTURE_HPP
