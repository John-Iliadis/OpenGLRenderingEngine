//
// Created by Gianni on 22/01/2025.
//

#include "shader.hpp"

Shader::Shader()
    : mRendererId()
{
}

Shader::Shader(ShaderList shaderList)
{
    mRendererId = glCreateProgram();

    std::vector<uint32_t> compiledShaders;
    for (const auto& [shaderType, shaderPath] : shaderList)
    {
        std::string shaderSrc = parseShader(shaderPath);
        uint32_t shaderID = compileShader(shaderType, shaderSrc);
        glAttachShader(mRendererId, shaderID);
        compiledShaders.push_back(shaderID);
    }

    glLinkProgram(mRendererId);

    for (uint32_t shaderID : compiledShaders)
        glDeleteShader(shaderID);

    queryUniforms();
}

Shader::~Shader()
{
    glDeleteProgram(mRendererId);
}

Shader::Shader(Shader &&other) noexcept
{
    mRendererId = other.mRendererId;
    mUniformLocationCache = std::move(other.mUniformLocationCache);

    other.mRendererId = 0;
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        glDeleteProgram(mRendererId);

        mRendererId = other.mRendererId;
        mUniformLocationCache = std::move(other.mUniformLocationCache);

        other.mRendererId = 0;
    }

    return *this;
}

void Shader::bind() const
{
    glUseProgram(mRendererId);
}

void Shader::unbind() const
{
    glUseProgram(0);
}

void Shader::setInt(const std::string& name, int v0) const
{
    glUniform1i(getUniformLocation(name), v0);
}

void Shader::setFloat(const std::string& name, float v0) const
{
    glUniform1f(getUniformLocation(name), v0);
}

void Shader::setFloat2(const std::string& name, float v0, float v1) const
{
    glUniform2f(getUniformLocation(name), v0, v1);
}

void Shader::setFloat2(const std::string& name, const glm::vec2 &vec2) const
{
    glUniform2f(getUniformLocation(name), vec2.x, vec2.y);
}

void Shader::setFloat2(const std::string& name, float v0, float v1, float v2) const
{
    glUniform3f(getUniformLocation(name), v0, v1, v2);
}

void Shader::setFloat3(const std::string& name, const glm::vec3 &vec3) const
{
    glUniform3f(getUniformLocation(name), vec3.x, vec3.y, vec3.z);
}

void Shader::setFloat4(const std::string& name, float v0, float v1, float v2, float v3) const
{
    glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::setFloat4(const std::string& name, const glm::vec4 &vec4) const
{
    glUniform4f(getUniformLocation(name), vec4.x, vec4.y, vec4.z, vec4.w);
}

void Shader::setMat4(const std::string& name, const glm::mat4 &matrix) const
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

uint32_t Shader::id() const
{
    return mRendererId;
}

std::string Shader::parseShader(const std::string& path)
{
    std::ifstream file(path);
    check(file.is_open(), "Failed to open shader file.");

    std::ostringstream oss;
    oss << file.rdbuf();

    return oss.str();
}

uint32_t Shader::compileShader(uint32_t type, const std::string& shaderSrc)
{
    uint32_t shaderId = glCreateShader(type);

    const char* src = shaderSrc.c_str();
    glShaderSource(shaderId, 1, &src, nullptr);
    glCompileShader(shaderId);

    return shaderId;
}

void Shader::queryUniforms()
{
    int uniformCount;
    glGetProgramiv(mRendererId, GL_ACTIVE_UNIFORMS, &uniformCount);

    char buffer[128];
    for (int i = 0; i < uniformCount; ++i)
    {
        glGetActiveUniform(mRendererId, i, sizeof(buffer), nullptr, nullptr, nullptr, buffer);
        mUniformLocationCache.emplace(std::string(buffer), glGetUniformLocation(mRendererId, buffer));
    }
}

int Shader::getUniformLocation(const std::string& name) const
{
    if (mUniformLocationCache.contains(name))
        return mUniformLocationCache.at(name);

    debugLog(std::format("Uniform {} doesn't exist or it may not be used.", name));
    assert(false);
}
