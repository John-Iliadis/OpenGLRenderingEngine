//
// Created by Gianni on 22/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPEN_GL_SHADER_HPP
#define OPENGLRENDERINGENGINE_OPEN_GL_SHADER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../utils.hpp"

using ShaderPath = std::string;
using ShaderList = std::initializer_list<std::pair<GLenum, ShaderPath>>;

class OpenGLShader
{
public:
    OpenGLShader();
    OpenGLShader(ShaderList shaderList);
    ~OpenGLShader();

    OpenGLShader(OpenGLShader&& other) noexcept;
    OpenGLShader& operator=(OpenGLShader&& other) noexcept;

    OpenGLShader(const OpenGLShader&) = delete;
    OpenGLShader& operator=(const OpenGLShader&) = delete;

    void bind() const;
    void unbind() const;

    void setInt(const std::string& name, int v0) const;
    void setFloat(const std::string& name, float v0) const;
    void setFloat2(const std::string& name, float v0 , float v1) const;
    void setFloat2(const std::string& name, const glm::vec2& vec2) const;
    void setFloat2(const std::string& name, float v0, float v1, float v2) const;
    void setFloat3(const std::string& name, const glm::vec3& vec3) const;
    void setFloat4(const std::string& name, float v0, float v1, float v2, float v3) const;
    void setFloat4(const std::string& name, const glm::vec4& vec4) const;
    void setMat4(const std::string& name, const glm::mat4& matrix) const;

    uint32_t id() const;

private:
    std::string parseShader(const std::string& path);
    uint32_t compileShader(uint32_t type, const std::string& shaderSrc);
    void queryUniforms();
    int getUniformLocation(const std::string& name) const;

private:
    uint32_t mRendererId;
    std::unordered_map<std::string, int> mUniformLocationCache;
};

#endif //OPENGLRENDERINGENGINE_OPEN_GL_SHADER_HPP
