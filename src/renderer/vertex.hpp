//
// Created by Gianni on 1/02/2025.
//

#ifndef OPENGLRENDERINGENGINE_VERTEX_HPP
#define OPENGLRENDERINGENGINE_VERTEX_HPP

#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

#endif //OPENGLRENDERINGENGINE_VERTEX_HPP
