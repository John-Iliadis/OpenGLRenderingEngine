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

GLenum toGLenum(TextureDataType dataType)
{
    switch (dataType)
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

std::shared_ptr<uint8_t> loadImage(const std::string& imagePath, int32_t* width, int32_t* height, int32_t requiredChannels)
{
    uint8_t* data = stbi_load(imagePath.c_str(), width, height, nullptr, requiredChannels);
    check(data, std::format("Failed to load {}", imagePath).c_str());

    return std::shared_ptr<uint8_t>(data, [] (uint8_t* d) { stbi_image_free(d); });
}

std::shared_ptr<uint8_t> loadImageHDR(const std::string& imagePath, int32_t* width, int32_t* height, int32_t requiredChannels)
{
    float* data = stbi_loadf(imagePath.c_str(), width, height, nullptr, requiredChannels);
    check(data, std::format("Failed to load {}", imagePath).c_str());

    return std::shared_ptr<uint8_t>((uint8_t*)data, [] (uint8_t* d) { stbi_image_free(d); });
}

int32_t getRequiredChannels(TextureFormat format)
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

// -- OpenGLTexture -- //

Texture::Texture()
    : mRendererID()
    , mSpecification()
{
}

Texture::Texture(const TextureSpecification &spec)
    : mRendererID()
    , mSpecification(spec)
{
}

Texture::~Texture()
{
    glDeleteTextures(1, &mRendererID);
}

Texture::Texture(Texture &&other) noexcept
{
    std::swap(mRendererID, other.mRendererID);
    std::swap(mSpecification, other.mSpecification);
}

Texture &Texture::operator=(Texture &&other) noexcept
{
    if (this != &other)
    {
        glDeleteTextures(1, &mRendererID);

        std::swap(mRendererID, other.mRendererID);
        std::swap(mSpecification, other.mSpecification);
    }

    return *this;
}

void Texture::bind(uint32_t unit)
{
    glBindTextureUnit(unit, mRendererID);
}

void Texture::unbind(uint32_t unit)
{
    glBindTextureUnit(unit, 0);
}

uint32_t Texture::id() const
{
    return mRendererID;
}

int32_t Texture::width() const
{
    return mSpecification.width;
}

int32_t Texture::height() const
{
    return mSpecification.height;
}

// -- OpenGLTexture2D -- //

Texture2D::Texture2D(const TextureSpecification &spec)
    : Texture(spec)
{
    create();
}

Texture2D::Texture2D(const TextureSpecification &spec, const void *textureData)
    : Texture(spec)
{
    create();
    uploadTextureData(textureData);
}

Texture2D::Texture2D(const TextureSpecification &spec, const std::string &texturePath)
    : Texture(spec)
{
    int32_t requiredChannels = getRequiredChannels(mSpecification.format);

    std::shared_ptr<uint8_t> textureData;
    if (spec.dataType == TextureDataType::FLOAT)
        textureData = loadImageHDR(texturePath, &mSpecification.width, &mSpecification.height, requiredChannels);
    else
        textureData = loadImage(texturePath, &mSpecification.width, &mSpecification.height, requiredChannels);

    create();
    uploadTextureData(textureData.get());
}

void Texture2D::resize(int32_t width, int32_t height)
{
    mSpecification.width = width;
    mSpecification.height = height;

    glDeleteTextures(1, &mRendererID);

    create();
}

void Texture2D::create()
{
    glCreateTextures(GL_TEXTURE_2D, 1, &mRendererID);
    glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_S, toGLenum(mSpecification.wrapMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_T, toGLenum(mSpecification.wrapMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_MIN_FILTER, toGLenumMinFilter(mSpecification.filterMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_MAG_FILTER, toGLenumMinFilter(mSpecification.filterMode));

    if (mSpecification. filterMode == TextureFilter::Anisotropic)
    {
        float maxAnisotropy = 0.f;
        glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        glTextureParameterf(mRendererID, GL_TEXTURE_MAX_ANISOTROPY, glm::max(16.f, maxAnisotropy));
    }

    uint32_t mipLevels = 0;
    if (mSpecification.generateMipMaps)
        mipLevels = calculateMipLevels(mSpecification.width, mSpecification.height);

    glTextureStorage2D(mRendererID,
                       mipLevels,
                       toGLenum(mSpecification.format),
                       mSpecification.width,
                       mSpecification.height);
}

void Texture2D::uploadTextureData(const void *textureData)
{
    glTextureSubImage2D(mRendererID,
                        0,
                        0, 0,
                        mSpecification.width,
                        mSpecification.height,
                        toGLenum(mSpecification.format),
                        toGLenum(mSpecification.dataType),
                        textureData);
}

// -- OpenGLTexture2DMultisample -- //

Texture2DMultisample::Texture2DMultisample(const TextureSpecification &spec, int32_t sampleCount)
    : Texture(spec)
{
    int32_t maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    mSampleCount = glm::max(sampleCount, maxSamples);

    create();
}

void Texture2DMultisample::resize(int32_t width, int32_t height)
{
    mSpecification.width = width;
    mSpecification.height = height;

    glDeleteTextures(1, &mRendererID);

    create();
}

void Texture2DMultisample::create()
{
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &mRendererID);
    glTextureStorage2DMultisample(mRendererID,
                                  mSampleCount,
                                  toGLenum(mSpecification.format),
                                  mSpecification.width,
                                  mSpecification.height,
                                  GL_FALSE);
}

// -- OpenGLTextureCube -- //

TextureCube::TextureCube(const TextureSpecification &spec)
    : Texture(spec)
{
    create();
}

TextureCube::TextureCube(const TextureSpecification &spec, const void **textureData)
    : TextureCube(spec)
{
    create();
    for (int32_t i = 0; i < 6; ++i)
        uploadTextureData(i, textureData[i]);
}

TextureCube::TextureCube(const TextureSpecification &spec, const std::array<std::string, 6> &texturePaths)
    : Texture(spec)
{
    create();

    int32_t requiredChannels = getRequiredChannels(mSpecification.format);
    for (uint32_t i = 0; i < 6; ++i)
    {
        std::shared_ptr<uint8_t> textureData;
        if (spec.dataType == TextureDataType::FLOAT)
            textureData = loadImageHDR(texturePaths.at(i), &mSpecification.width, &mSpecification.height, requiredChannels);
        else
            textureData = loadImage(texturePaths.at(i), &mSpecification.width, &mSpecification.height, requiredChannels);

        uploadTextureData(i, textureData.get());
    }
}

void TextureCube::create()
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mRendererID);
    glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_S, toGLenum(mSpecification.wrapMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_T, toGLenum(mSpecification.wrapMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_R, toGLenum(mSpecification.wrapMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_MIN_FILTER, toGLenumMinFilter(mSpecification.filterMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_MAG_FILTER, toGLenumMinFilter(mSpecification.filterMode));

    if (mSpecification. filterMode == TextureFilter::Anisotropic)
    {
        float maxAnisotropy = 0.f;
        glGetTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        glTextureParameterf(mRendererID, GL_TEXTURE_MAX_ANISOTROPY, glm::max(16.f, maxAnisotropy));
    }

    uint32_t mipLevels = 0;
    if (mSpecification.generateMipMaps)
        mipLevels = calculateMipLevels(mSpecification.width, mSpecification.height);

    glTextureStorage2D(mRendererID,
                       mipLevels,
                       toGLenum(mSpecification.format),
                       mSpecification.width,
                       mSpecification.height);
}

void TextureCube::uploadTextureData(int32_t faceIndex, const void* textureData)
{
    glTextureSubImage3D(mRendererID,
                        0,
                        0, 0, faceIndex,
                        mSpecification.width,
                        mSpecification.height,
                        1,
                        toGLenum(mSpecification.format),
                        toGLenum(mSpecification.dataType),
                        textureData);
}
