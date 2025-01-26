//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP
#define OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP

#include <glad/glad.h>
#include "../opengl/shader.hpp"
#include "../renderer/material.hpp"
#include "resource_importer.hpp"

class Editor;
class Renderer;

using ModelMetaData = std::string;
using MaterialMetaData = std::string;
using TextureMetaData = std::string;
using MeshMetaData = std::string;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    void importModel(const std::filesystem::path& path);

    void processMainThreadTasks();

private:
    void addModel(std::shared_ptr<LoadedModelData> loadedModel);

    void deleteModel(const std::filesystem::path& path);
    void deleteMaterial(uint32_t materialIndex);
    void deleteTexture(std::filesystem::path& path);

    void loadDefaultMaterial();

private:
    std::unordered_map<std::filesystem::path, std::shared_ptr<Model>> mModels;
    std::unordered_map<std::shared_ptr<Model>, ModelMetaData> mModelMetaData;

    std::vector<std::shared_ptr<InstancedMesh>> mMeshes;
    std::unordered_map<std::shared_ptr<InstancedMesh>, MeshMetaData> mMeshMetaData;

    std::unordered_map<std::filesystem::path, std::shared_ptr<Texture2D>> mTextures;
    std::unordered_map<std::shared_ptr<Texture2D>, TextureMetaData> mTextureMetaData;
    std::unordered_map<std::shared_ptr<Texture2D>, uint64_t> mBindlessTextureMap;
    std::vector<uint64_t> mBindlessTextures;
    ShaderBuffer mBindlessTextureSSBO;

    std::vector<Material> mMaterials;
    std::unordered_map<Material*, MaterialMetaData> mMaterialMetaData;
    ShaderBuffer mMaterialsSSBO;

    std::vector<std::future<std::shared_ptr<LoadedModelData>>> mLoadedModelFutures;
    MainThreadTaskQueue mTaskQueue;

private:
    friend class Editor;
    friend class Renderer;
};

#endif //OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP
