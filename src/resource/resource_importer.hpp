//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_RESOURCE_IMPORTER_HPP
#define OPENGLRENDERINGENGINE_RESOURCE_IMPORTER_HPP

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../utils.hpp"
#include "loaded_resource.hpp"

using EnqueueCallback = std::function<void(std::function<void()>&&)>;

namespace ResourceImporter
{
    // todo: fix scene graph
    // todo: fix fbx texture paths
    // todo: fix gltf texture paths
    std::future<std::shared_ptr<LoadedModelData>> loadModel(const std::filesystem::path& path, EnqueueCallback callback);

    LoadedModelData::Mesh createMesh(const MeshData& meshData);

    std::pair<std::filesystem::path, std::shared_ptr<Texture2D>> createTexturePair(const LoadedImage& loadedImage);

    std::shared_ptr<aiScene> loadAssimpScene(const std::filesystem::path& path);

    Model::Node createModelGraph(const aiNode& assimpNode);

    std::future<MeshData> createMeshData(const aiMesh& assimpMesh);

    std::future<std::shared_ptr<LoadedImage>> createTextureData(const std::filesystem::path& path);

    LoadedModelData::Material createMaterial(const aiMaterial& assimpMaterial, const std::filesystem::path& directory);

    std::vector<InstancedMesh::Vertex> loadMeshVertices(const aiMesh& mesh);

    std::vector<uint32_t> loadMeshIndices(const aiMesh& mesh);

    glm::mat4 assimpToGlmMat4(const aiMatrix4x4& matrix);

    std::string getTextureName(const aiMaterial& material, aiTextureType textureType);
}

#endif //OPENGLRENDERINGENGINE_RESOURCE_IMPORTER_HPP
