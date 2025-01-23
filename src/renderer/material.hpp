//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MATERIAL_HPP
#define OPENGLRENDERINGENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

enum class Workflow : uint32_t
{
    Specular = 0,
    Metallic = 1
};

struct Material
{
    Workflow workflow;
    uint32_t albedoMapIndex;
    uint32_t specularMapIndex;
    uint32_t roughnessMapIndex;
    uint32_t metallicMapIndex;
    uint32_t normalMapIndex;
    uint32_t displacementMapIndex;
    uint32_t aoMapIndex;
    uint32_t emissionMapIndex;
    glm::vec4 albedoColor;
    glm::vec4 emissionColor;
    glm::vec2 tiling;
    glm::vec2 offset;
};

#endif //OPENGLRENDERINGENGINE_MATERIAL_HPP
