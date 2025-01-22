//
// Created by Gianni on 22/01/2025.
//

#include "opengl_texture2D.hpp"


OpenGLTexture2D::OpenGLTexture2D()
    : mRendererID()
    , mSpecification()
{
}

OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &spec)
    : mRendererID()
    , mSpecification(spec)
{
    create();
}

OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &spec, const void *textureData)
    : mRendererID()
    , mSpecification(spec)
{
    create();
    uploadTextureData(textureData);
}

OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &spec, const std::string &texturePath)
    : mRendererID()
    , mSpecification(spec)
{
    int requiredChannels = getRequiredChannels(mSpecification.format);

    if (spec.dataType == TextureDataType::FLOAT)
    {
        std::shared_ptr<float> textureData = loadImageHDR(texturePath, &mSpecification.width, &mSpecification.height, requiredChannels);
        create();
        uploadTextureData(textureData.get());
    }
    else
    {
        std::shared_ptr<uint8_t> textureData = loadImage(texturePath, &mSpecification.width, &mSpecification.height, requiredChannels);
        create();
        uploadTextureData(textureData.get());
    }
}

OpenGLTexture2D::~OpenGLTexture2D()
{
    destroy();
}

OpenGLTexture2D::OpenGLTexture2D(OpenGLTexture2D &&other) noexcept
    : OpenGLTexture2D()
{
    std::swap(mRendererID, other.mRendererID);
    std::swap(mSpecification, other.mSpecification);
}

OpenGLTexture2D &OpenGLTexture2D::operator=(OpenGLTexture2D &&other) noexcept
{
    if (this != &other)
    {
        destroy();

        std::swap(mRendererID, other.mRendererID);
        std::swap(mSpecification, other.mSpecification);

        other = OpenGLTexture2D();
    }

    return *this;
}

void OpenGLTexture2D::resize(int32_t width, int32_t height)
{
    mSpecification.width = width;
    mSpecification.height = height;

    destroy();
    create();
}

void OpenGLTexture2D::bind(uint32_t unit)
{
    glBindTextureUnit(unit, mRendererID);
}

void OpenGLTexture2D::unbind(uint32_t unit)
{
    glBindTextureUnit(unit, 0);
}

uint32_t OpenGLTexture2D::id() const
{
    return mRendererID;
}

int32_t OpenGLTexture2D::width() const
{
    return mSpecification.width;
}

int32_t OpenGLTexture2D::height() const
{
    return mSpecification.width;
}

void OpenGLTexture2D::create()
{
    glCreateTextures(GL_TEXTURE_2D, 1, &mRendererID);
    setTextureParameters(mSpecification.wrapMode, mSpecification.filterMode);

    uint32_t mipLevels = 0;
    if (mSpecification.generateMipMaps)
        mipLevels = calculateMipLevels(mSpecification.width, mSpecification.height);

    glTextureStorage2D(mRendererID,
                       mipLevels,
                       toGLenum(mSpecification.format),
                       mSpecification.width,
                       mSpecification.height);
}

void OpenGLTexture2D::destroy()
{
    if (mRendererID)
    {
        glDeleteTextures(1, &mRendererID);
    }
}

void OpenGLTexture2D::setTextureParameters(TextureWrap wrapMode, TextureFilter filterMode)
{
    glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_S, toGLenum(wrapMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_T, toGLenum(wrapMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_MIN_FILTER, toGLenumMinFilter(filterMode));
    glTextureParameteri(mRendererID, GL_TEXTURE_MAG_FILTER, toGLenumMinFilter(filterMode));

    if (filterMode == TextureFilter::Anisotropic)
    {
        float maxAnisotropy = 0.f;
        glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        glTextureParameterf(mRendererID, GL_TEXTURE_MAX_ANISOTROPY, glm::max(16.f, maxAnisotropy));
    }
}

void OpenGLTexture2D::uploadTextureData(const void *textureData)
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
