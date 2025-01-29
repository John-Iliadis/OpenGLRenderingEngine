//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MATERIAL_HPP
#define OPENGLRENDERINGENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

enum DefaultTextureIndices
{
    DefaultBaseColorWhite = 0,
    DefaultBaseColorGrey,
    DefaultMetallicRoughness,
    DefaultNormal,
    DefaultAo,
    DefaultEmission,
    DefaultSpecular,
    DefaultDisplacement,
    DefaultTextureCount
};

enum MaterialTextureIndices
{
    BaseColor = 0,
    MetallicRoughness,
    Normal,
    Ao,
    Emission,
    Specular,
    Displacement,
    MaterialTextureCount
};

inline const std::unordered_map<int32_t, int32_t> gDefaultTextureMap {
    {BaseColor, DefaultBaseColorWhite},
    {MetallicRoughness, DefaultMetallicRoughness},
    {Normal, DefaultNormal},
    {Ao, DefaultAo},
    {Emission, DefaultEmission},
    {Specular, DefaultSpecular},
    {Displacement, DefaultDisplacement}
};

enum Workflow
{
    MetallicWorkflow = 0,
    SpecularWorkflow = 1
};

struct Material
{
    int32_t workflow;
    int32_t textures[MaterialTextureCount];
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
        : workflow(MetallicWorkflow)
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
        for (int32_t i = 0; i < MaterialTextureCount; ++i)
            textures[i] = gDefaultTextureMap.at(i);
    }
};

#endif //OPENGLRENDERINGENGINE_MATERIAL_HPP
