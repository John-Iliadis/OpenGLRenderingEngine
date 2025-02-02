//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MATERIAL_HPP
#define OPENGLRENDERINGENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

enum class MatTexType
{
    BaseColor,
    MetallicRoughness,
    Normal,
    Ao,
    Emission,
};

struct Material
{
    alignas(4) uint32_t baseColorTexIndex;
    alignas(4) uint32_t metallicRoughnessTexIndex;
    alignas(4) uint32_t normalTexIndex;
    alignas(4) uint32_t aoTexIndex;
    alignas(4) uint32_t emissionTexIndex;
    alignas(16) glm::vec4 baseColorFactor;
    alignas(16) glm::vec4 emissionFactor;
    alignas(4) float metallicFactor;
    alignas(4) float roughnessFactor;
    alignas(4) float occlusionFactor;
    alignas(8) glm::vec2 tiling;
    alignas(8) glm::vec2 offset;
};

inline constexpr uint32_t DefaultBaseColorTexIndex = 0;
inline constexpr uint32_t DefaultMetallicRoughnessTexIndex = 1;
inline constexpr uint32_t DefaultNormalTexIndex = 2;
inline constexpr uint32_t DefaultAoTexIndex = 3;
inline constexpr uint32_t DefaultEmissionTexIndex = 4;

#endif //OPENGLRENDERINGENGINE_MATERIAL_HPP
