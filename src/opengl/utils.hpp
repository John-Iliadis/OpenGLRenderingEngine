//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_UTILS_HPP
#define OPENGLRENDERINGENGINE_UTILS_HPP

#include <glad/glad.h>

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char* message,
                            const void* userParam);

#endif //OPENGLRENDERINGENGINE_UTILS_HPP
