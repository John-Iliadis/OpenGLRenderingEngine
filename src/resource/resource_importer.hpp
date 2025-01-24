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
    std::future<std::shared_ptr<LoadedModel>> loadModel(const std::filesystem::path& path, EnqueueCallback callback);

    LoadedModel::Mesh createMesh(const MeshData& meshData);

    std::pair<std::filesystem::path, Texture2D> createTexturePair(const ImageLoader& loadedImage);

    std::shared_ptr<aiScene> loadAssimpScene(const std::filesystem::path& path);

    Model::Node createModelGraph(const aiNode& assimpNode);

    std::future<MeshData> createMeshData(const aiMesh& assimpMesh);

    std::future<std::shared_ptr<ImageLoader>> createTextureData(const std::filesystem::path& path);

    LoadedModel::Material createMaterial(const aiMaterial& assimpMaterial, const std::filesystem::path& directory);

    std::vector<InstancedMesh::Vertex> loadMeshVertices(const aiMesh& mesh);

    std::vector<uint32_t> loadMeshIndices(const aiMesh& mesh);

    glm::mat4 assimpToGlmMat4(const aiMatrix4x4& matrix);

    std::string getTextureName(const aiMaterial& material, aiTextureType textureType);
}

#endif //OPENGLRENDERINGENGINE_RESOURCE_IMPORTER_HPP
