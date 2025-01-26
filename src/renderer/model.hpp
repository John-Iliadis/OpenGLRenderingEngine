//
// Created by Gianni on 24/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MODEL_HPP
#define OPENGLRENDERINGENGINE_MODEL_HPP

#include <glm/glm.hpp>
#include "../app/simple_notification_service.hpp"
#include "instanced_mesh.hpp"

class Model
{
public:
    struct Node
    {
        std::string name;
        glm::mat4 model;
        std::vector<uint32_t> meshes;
        std::vector<Node> children;
    };

    struct Mesh
    {
        std::shared_ptr<InstancedMesh> mesh;
        uint32_t materialIndex;
    };

public:
    Node root;
    std::vector<Mesh> meshes;
    std::unordered_map<uint32_t, std::string> indirectMatIndexMap;
    std::unordered_map<std::string, uint32_t> matNameToMatIndex;

public:
    void remapMaterial(const std::string& name, uint32_t materialIndex)
    {
        matNameToMatIndex.at(name) = materialIndex;
    }

    uint32_t getMaterialIndex(uint32_t materialIndex)
    {
        return matNameToMatIndex.at(indirectMatIndexMap.at(materialIndex));
    }
};

#endif //OPENGLRENDERINGENGINE_MODEL_HPP
