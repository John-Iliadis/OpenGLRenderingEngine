//
// Created by Gianni on 22/01/2025.
//

#include "texture.hpp"
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

    if (!data)
    {
        debugLog(std::format("loadImage: Failed to load {}", imagePath));
        return nullptr;
    }

    return std::shared_ptr<uint8_t>(data, [] (uint8_t* d) { stbi_image_free(d); });
}

std::shared_ptr<uint8_t> loadImageHDR(const std::string& imagePath, int32_t* width, int32_t* height, int32_t requiredChannels)
{
    float* data = stbi_loadf(imagePath.c_str(), width, height, nullptr, requiredChannels);

    if (!data)
    {
        debugLog(std::format("loadImage: Failed to load {}", imagePath));
        return nullptr;
    }

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

// -- ImageLoader -- //

ImageLoader::ImageLoader()
    : mSuccess()
    , mWidth()
    , mHeight()
    , mComponents()
    , mFormat()
    , mDataType()
    , mData()
{
}

ImageLoader::ImageLoader(const std::filesystem::path &imagePath, int32_t requiredComponents)
{
    load(imagePath, requiredComponents);
}

ImageLoader::~ImageLoader() { clear(); }

ImageLoader::ImageLoader(ImageLoader &&other) noexcept
    : ImageLoader()
{
    swap(other);
}

ImageLoader &ImageLoader::operator=(ImageLoader &&other) noexcept
{
    if (this != &other)
    {
        clear();
        swap(other);
    }

    return *this;
}

void ImageLoader::load(const std::filesystem::path &imagePath, int32_t requiredComponents)
{
    clear();

    mPath = imagePath;
    bool isHDR = stbi_is_hdr(imagePath.string().c_str());

    if (isHDR)
    {
        mData = stbi_loadf(imagePath.string().c_str(), &mWidth, &mHeight, &mComponents, requiredComponents);
    }
    else
    {
        mData = stbi_load(imagePath.string().c_str(), &mWidth, &mHeight, &mComponents, requiredComponents);
    }

    if (!mData)
        return;

    if (isHDR)
    {
        mDataType = TextureDataType::FLOAT;

        switch (mComponents)
        {
            case 1: mFormat = TextureFormat::R8; break;
            case 3: mFormat = TextureFormat::RGB8; break;
            case 4: mFormat = TextureFormat::RGBA8; break;
            default: check(false, "Case not supported.");
        }
    }
    else
    {
        mDataType = TextureDataType::UINT8;

        switch (mComponents)
        {
            case 3: mFormat = TextureFormat::RGB32F; break;
            case 4: mFormat = TextureFormat::RGBA32F; break;
            default: check(false, "Case not supported.");
        }
    }

    mSuccess = true;
}

void ImageLoader::swap(ImageLoader &other)
{
    std::swap(mPath, other.mPath);
    std::swap(mSuccess, other.mSuccess);
    std::swap(mWidth, other.mWidth);
    std::swap(mHeight, other.mHeight);
    std::swap(mComponents, other.mComponents);
    std::swap(mFormat, other.mFormat);
    std::swap(mDataType, other.mDataType);
    std::swap(mData, other.mData);
}

void ImageLoader::clear()
{
    mPath = "";
    mSuccess = false;
    mWidth = 0;
    mHeight = 0;
    mComponents = 0;
    stbi_image_free(mData);
    mData = nullptr;
}

const std::filesystem::path &ImageLoader::path() const
{
    return mPath;
}

bool ImageLoader::success() const
{
    return mSuccess;
}

int32_t ImageLoader::width() const
{
    return mWidth;
}

int32_t ImageLoader::height() const
{
    return mHeight;
}

int32_t ImageLoader::components() const
{
    return mComponents;
}

TextureFormat ImageLoader::format() const
{
    return mFormat;
}

TextureDataType ImageLoader::dataType() const
{
    return mDataType;
}

void *ImageLoader::data() const
{
    return mData;
}

// -- Texture -- //

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
    mRendererID = other.mRendererID;
    mSpecification = other.mSpecification;

    other.mRendererID = 0;
    other.mSpecification = {};
}

Texture &Texture::operator=(Texture &&other) noexcept
{
    if (this != &other)
    {
        glDeleteTextures(1, &mRendererID);

        mRendererID = other.mRendererID;
        mSpecification = other.mSpecification;

        other.mRendererID = 0;
        other.mSpecification = {};
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

// -- Texture2D -- //

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

    check(static_cast<bool>(textureData), "Failed to load texture data.");

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

// -- Texture2DMultisample -- //

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

// -- TextureCube -- //

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

        check(static_cast<bool>(textureData), "Failed to load texture data.");

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
