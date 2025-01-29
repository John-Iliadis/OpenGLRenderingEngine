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

struct LoadedModelData
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
        int32_t baseColorMapIndex = -1;
        int32_t metallicRoughnessMapIndex = -1;
        int32_t normalMapIndex = -1;
        int32_t aoMapIndex = -1;
        int32_t emissionMapIndex = -1;
        int32_t specularGlossinessMapIndex = -1;
        int32_t displacementMapIndex = -1;
        glm::vec4 baseColorFactor = glm::vec4(1.f);
        glm::vec4 emissionFactor = glm::vec4(0.f);
        glm::vec4 specularGlossinessFactor = glm::vec4(1.f);
        float metallicFactor = 1.f;
        float roughnessFactor = 1.f;
        float occlusionFactor = 1.f;
        float specularFactor = 1.f;
    };

    std::filesystem::path path;
    std::string name;
    Model::Node root;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<std::pair<std::shared_ptr<Texture2D>, std::filesystem::path>> textures;
    std::unordered_map<int32_t, int32_t> indirectTextureMap;
};

#endif //OPENGLRENDERINGENGINE_LOADED_RESOURCE_HPP
