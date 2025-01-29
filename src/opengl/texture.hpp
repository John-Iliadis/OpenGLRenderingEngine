//
// Created by Gianni on 22/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_TEXTURE_HPP
#define OPENGLRENDERINGENGINE_TEXTURE_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include "../utils.hpp"

enum class TextureFormat
{
    R8,
    RG8,
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

GLenum toGLenumFormat(TextureFormat format);
GLenum toGLenumInternalFormat(TextureFormat format);
GLenum toGLenum(TextureDataType dataType);
GLenum toGLenum(TextureWrap wrapMode);
GLenum toGLenumMinFilter(TextureFilter filterMode);
GLenum toGLenumMagFilter(TextureFilter filterMode);

uint32_t calculateMipLevels(int32_t textureWidth, int32_t textureHeight);

int32_t getRequiredComponents(TextureFormat format);

class LoadedImage
{
public:
    LoadedImage();
    LoadedImage(const std::filesystem::path& imagePath, int32_t requiredComponents = 0);
    ~LoadedImage();

    LoadedImage(const LoadedImage&) = delete;
    LoadedImage& operator=(const LoadedImage&) = delete;

    LoadedImage(LoadedImage&& other) noexcept;
    LoadedImage& operator=(LoadedImage&& other) noexcept;

    void swap(LoadedImage& other);

    const std::filesystem::path& path() const;
    bool success() const;
    int32_t width() const;
    int32_t height() const;
    int32_t components() const;
    TextureFormat format() const;
    TextureDataType dataType() const;
    void* data() const;

private:
    std::filesystem::path mPath;
    bool mSuccess;
    int32_t mWidth;
    int32_t mHeight;
    int32_t mComponents;
    TextureFormat mFormat;
    TextureDataType mDataType;
    void* mData;
};

class Texture
{
public:
    Texture();
    Texture(const TextureSpecification& spec);
    virtual ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind(uint32_t unit);
    void unbind(uint32_t unit);

    uint32_t id() const;
    int32_t width() const;
    int32_t height() const;

protected:
    uint32_t mRendererID;
    TextureSpecification mSpecification;
};

class Texture2D : public Texture
{
public:
    Texture2D() = default;
    Texture2D(const TextureSpecification& spec);
    Texture2D(const TextureSpecification& spec, const void* textureData);
    Texture2D(const TextureSpecification& spec, const std::string& texturePath);

    void resize(int32_t width, int32_t height);

private:
    void create();
    void uploadTextureData(const void* textureData);
};

class Texture2DMultisample : public Texture
{
public:
    Texture2DMultisample() = default;
    Texture2DMultisample(const TextureSpecification& spec, int32_t sampleCount);

    void resize(int32_t width, int32_t height);

private:
    void create();
    int32_t mSampleCount;
};

class TextureCube : public Texture
{
public:
    TextureCube() = default;
    TextureCube(const TextureSpecification& spec);
    TextureCube(const TextureSpecification& spec, const void** textureData);
    TextureCube(const TextureSpecification& spec, const std::array<std::string, 6>& texturePaths);

private:
    void create();
    void uploadTextureData(int32_t faceIndex, const void* textureData);
};

#endif //OPENGLRENDERINGENGINE_TEXTURE_HPP
