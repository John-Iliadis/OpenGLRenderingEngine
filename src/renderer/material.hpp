//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MATERIAL_HPP
#define OPENGLRENDERINGENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

#define DEFAULT_MATERIAL_INDEX 0
#define BASE_COLOR_WHITE_TEXTURE_DEFAULT_INDEX 0
#define BASE_COLOR_GREY_TEXTURE_DEFAULT_INDEX 1
#define METALLIC_ROUGHNESS_TEXTURE_DEFAULT_INDEX 2
#define NORMAL_TEXTURE_DEFAULT_INDEX 3
#define AO_TEXTURE_DEFAULT_INDEX 4
#define EMISSION_TEXTURE_DEFAULT_INDEX 5
#define SPECULAR_TEXTURE_DEFAULT_INDEX 6
#define DISPLACEMENT_TEXTURE_DEFAULT_INDEX 7
#define DEFAULT_TEXTURE_COUNT 8

// material map indices
#define BASE_COLOR 0
#define METALLIC_ROUGHNESS 1
#define NORMAL 2
#define AO 3
#define EMISSION 4
#define SPECULAR 5
#define DISPLACEMENT 6
#define MATERIAL_TEXTURE_COUNT 7

#define METALLIC_WORKFLOW 0u
#define SPECULAR_WORKFLOW 1u

struct Material
{
    uint32_t workflow;
    uint32_t textures[MATERIAL_TEXTURE_COUNT];
    glm::vec4 baseColorFactor;
    glm::vec4 emissionFactor;
    glm::vec4 specularGlossinessFactor;
    float metallicFactor;
    float roughnessFactor;
    float occlusionFactor;
    float specularFactor;
    glm::vec2 tiling;
    glm::vec2 offset;

    Material()
        : workflow(METALLIC_WORKFLOW)
        , textures()
        , baseColorFactor(1.f)
        , emissionFactor(0.f)
        , specularGlossinessFactor(1.f)
        , metallicFactor(1.f)
        , roughnessFactor(1.f)
        , occlusionFactor(1.f)
        , specularFactor(1.f)
        , tiling(1.f)
        , offset(0.f)
    {
        textures[BASE_COLOR] = BASE_COLOR_WHITE_TEXTURE_DEFAULT_INDEX;
        textures[METALLIC_ROUGHNESS] = METALLIC_ROUGHNESS_TEXTURE_DEFAULT_INDEX;
        textures[NORMAL] = NORMAL_TEXTURE_DEFAULT_INDEX;
        textures[AO] = AO_TEXTURE_DEFAULT_INDEX;
        textures[EMISSION] = EMISSION_TEXTURE_DEFAULT_INDEX;
        textures[SPECULAR] = SPECULAR_TEXTURE_DEFAULT_INDEX;
        textures[DISPLACEMENT] = DISPLACEMENT_TEXTURE_DEFAULT_INDEX;
    }
};

inline const std::unordered_map<uint32_t, uint32_t> defaultTextureMap {
    {BASE_COLOR, BASE_COLOR_WHITE_TEXTURE_DEFAULT_INDEX},
    {METALLIC_ROUGHNESS, METALLIC_ROUGHNESS_TEXTURE_DEFAULT_INDEX},
    {NORMAL, NORMAL_TEXTURE_DEFAULT_INDEX},
    {AO, AO_TEXTURE_DEFAULT_INDEX},
    {EMISSION, EMISSION_TEXTURE_DEFAULT_INDEX},
    {SPECULAR, SPECULAR_TEXTURE_DEFAULT_INDEX},
    {DISPLACEMENT, DISPLACEMENT_TEXTURE_DEFAULT_INDEX}
};

#endif //OPENGLRENDERINGENGINE_MATERIAL_HPP
