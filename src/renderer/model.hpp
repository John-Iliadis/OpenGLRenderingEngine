//
// Created by Gianni on 24/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MODEL_HPP
#define OPENGLRENDERINGENGINE_MODEL_HPP

#include <glm/glm.hpp>
#include "instanced_mesh.hpp"

struct ModelNode
{
    std::string name;
    glm::mat4 model;
    std::vector<uint32_t> meshes;
    std::vector<ModelNode> children;
};

struct ModelMesh
{
    std::shared_ptr<InstancedMesh> mesh;
    std::string materialName;
};

struct Model
{
    std::string name;
    ModelNode root;
    std::vector<ModelMesh> meshes;

    // material name to resource material index
    std::unordered_map<std::string, uint32_t> mappedMaterials;
};

#endif //OPENGLRENDERINGENGINE_MODEL_HPP
