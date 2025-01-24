//
// Created by Gianni on 24/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MODEL_HPP
#define OPENGLRENDERINGENGINE_MODEL_HPP

#include <glm/glm.hpp>
#include "instanced_mesh.hpp"

struct Model
{
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
        std::string materialName;
    };

    std::string name;
    Node root;
    std::vector<Mesh> meshes;

    // material name to resource material index
    std::unordered_map<std::string, uint32_t> mappedMaterials;
};

#endif //OPENGLRENDERINGENGINE_MODEL_HPP
