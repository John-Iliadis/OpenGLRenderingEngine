//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MATERIAL_HPP
#define OPENGLRENDERINGENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

#define METALLIC_WORKFLOW 0
#define SPECULAR_WORKFLOW 1

// - metallic workflow uses the metallic roughness map
// - specular workflow uses the specular OR glossiness/specular map
// - for the specular workflow, the base color map will be the diffuse map

struct Material
{
    uint32_t workflow;
    uint32_t baseColorMapIndex;
    uint32_t metallicRoughnessMapIndex;
    uint32_t normalMapIndex;
    uint32_t aoMapIndex;
    uint32_t emissionMapIndex;
    uint32_t specularGlossinessMapIndex;
    uint32_t displacementMapIndex;
    glm::vec4 baseColorFactor = glm::vec4(1.f);
    glm::vec4 emissionFactor = glm::vec4(0.f);
    glm::vec4 specularGlossinessFactor = glm::vec4(1.f);
    float metallicFactor = 1.f;
    float roughnessFactor = 1.f;
    float occlusionFactor = 1.f;
    float specularFactor = 1.f;
    glm::vec2 tiling = glm::vec2(1.f);
    glm::vec2 offset = glm::vec2(0.f);
};

#endif //OPENGLRENDERINGENGINE_MATERIAL_HPP
