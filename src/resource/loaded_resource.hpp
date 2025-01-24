//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_LOADED_RESOURCE_HPP
#define OPENGLRENDERINGENGINE_LOADED_RESOURCE_HPP

#include "../renderer/instanced_mesh.hpp"
#include "../renderer/model.hpp"
#include "../opengl/texture.hpp"
#include "../opengl/buffer.hpp"

struct MeshData
{
    std::string name;
    std::vector<InstancedMesh::Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t materialIndex;
};

struct LoadedModel
{
    struct Mesh
    {
        std::string name;
        std::shared_ptr<InstancedMesh> mesh;
        uint32_t materialIndex;
    };

    struct Material
    {
        std::string name;
        std::filesystem::path albedoTexturePath;
        std::filesystem::path specularTexturePath;
        std::filesystem::path roughnessTexturePath;
        std::filesystem::path metallicTexturePath;
        std::filesystem::path normalTexturePath;
        std::filesystem::path displacementTexturePath;
        std::filesystem::path aoTexturePath;
        std::filesystem::path emissionTexturePath;
        glm::vec4 albedoColor;
        glm::vec4 emissionColor;
    };

    std::string modelName;
    Model::Node root;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::unordered_map<std::filesystem::path, Texture2D> textures;
};

#endif //OPENGLRENDERINGENGINE_LOADED_RESOURCE_HPP
