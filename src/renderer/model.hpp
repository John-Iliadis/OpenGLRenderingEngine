//
// Created by Gianni on 24/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MODEL_HPP
#define OPENGLRENDERINGENGINE_MODEL_HPP

#include <glm/glm.hpp>
#include "instanced_mesh.hpp"

class Model
{
public:
    struct Node
    {
        std::string name;
        glm::mat4 transformation;
        std::optional<uint32_t> mesh;
        std::vector<uint32_t> children;
    };

    struct Mesh
    {
        uint32_t meshIndex;
        uint32_t materialIndex;
    };

public:
    Node root;
    std::vector<Mesh> meshes;
    std::unordered_map<uint32_t, uint32_t> indirectMeshMap;
    std::unordered_map<uint32_t, std::string> indirectMatIndexMap;
    std::unordered_map<std::string, uint32_t> mappedMaterials;

public:
    void remapMaterial(const std::string& name, uint32_t materialIndex)
    {
        mappedMaterials.at(name) = materialIndex;
    }

    uint32_t getMaterialIndex(uint32_t meshIndex) const
    {
        return mappedMaterials.at(indirectMatIndexMap.at(meshes.at(meshIndex).materialIndex));
    }

    uint32_t getMeshIndex(uint32_t meshIndex) const
    {
        return indirectMeshMap.at(meshIndex);
    }
};

#endif //OPENGLRENDERINGENGINE_MODEL_HPP
