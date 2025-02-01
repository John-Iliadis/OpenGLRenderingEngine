//
// Created by Gianni on 24/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MODEL_HPP
#define OPENGLRENDERINGENGINE_MODEL_HPP

#include <glm/glm.hpp>
#include "bounding_box.hpp"
#include "instanced_mesh.hpp"

class Model
{
public:
    struct Node
    {
        std::string name;
        glm::mat4 transformation;
        std::optional<uint32_t> mesh;
        std::vector<Node> children;
    };

    struct Mesh
    {
        uint32_t meshIndex; // todo: remove this
        uint32_t materialIndex;
    };

public:
    Node root;
    std::vector<Mesh> meshes;
    BoundingBox bb;
    std::unordered_map<uint32_t, uint32_t> indirectMeshMap; // Model::Mesh::meshIndex -> ResourceManager::mMeshes (meshID)
    std::unordered_map<uint32_t, std::string> indirectMaterialMap; // Model::Mesh::materialIndex -> Model::mappedMaterials
    std::unordered_map<std::string, uint32_t> mappedMaterials; // mappedMaterials -> ResourceManager::mMaterials (index)

public:
    void remapMaterial(const std::string& name, uint32_t materialIndex)
    {
        mappedMaterials.at(name) = materialIndex;
    }

    uint32_t getMaterialIndex(const Model::Node& node) const
    {
        return mappedMaterials.at(indirectMaterialMap.at(meshes.at(node.mesh.value()).materialIndex));
    }

    uint32_t getMeshID(const Model::Node& node) const
    {
        return indirectMeshMap.at(meshes.at(node.mesh.value()).meshIndex);
    }

    uint32_t getMeshID(const Model::Mesh& mesh) const
    {
        return indirectMeshMap.at(mesh.meshIndex);
    }
};

#endif //OPENGLRENDERINGENGINE_MODEL_HPP
