//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP
#define OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP

#include <glad/glad.h>
#include "../opengl/shader.hpp"
#include "../renderer/material.hpp"
#include "../app/simple_notification_service.hpp"
#include "resource_importer.hpp"

class Editor;
class Renderer;

using ModelMetaData = std::string;
using MaterialMetaData = std::string;
using TextureMetaData = std::string;
using MeshMetaData = std::string;

uint32_t getModelID(const std::shared_ptr<Model> model);
uint32_t getMeshID(const std::shared_ptr<InstancedMesh> mesh);
uint32_t getTextureID(const std::shared_ptr<Texture2D> texture);

class ResourceManager : public SubscriberSNS
{
public:
    ResourceManager();
    ~ResourceManager();

    bool importModel(const std::filesystem::path& path);

    void notify(const Message &message) override;

    void processMainThreadTasks();

private:
    void addModel(std::shared_ptr<LoadedModelData> modelData);

    void deleteModel(uint32_t modelID);
    void deleteTexture(uint32_t textureID);
    void deleteMaterial(uint32_t removeIndex);

    void loadDefaultTextures();
    void loadDefaultMaterial();

private:
    // All models
    std::unordered_map<uint32_t, std::shared_ptr<Model>> mModels;
    std::unordered_map<uint32_t, ModelMetaData> mModelMetaData;
    std::unordered_map<uint32_t, std::filesystem::path> mModelPaths;

    // All meshes
    std::unordered_map<uint32_t, std::shared_ptr<InstancedMesh>> mMeshes;
    std::unordered_map<uint32_t, MeshMetaData> mMeshMetaData;

    // All textures
    std::unordered_map<uint32_t, std::shared_ptr<Texture2D>> mTextures;
    std::unordered_map<uint32_t, TextureMetaData> mTextureMetaData;
    std::unordered_map<uint32_t, std::filesystem::path> mTexturePaths;
    std::unordered_map<uint32_t, uint32_t> mBindlessTextureMap;
    std::vector<uint64_t> mBindlessTextures;
    ShaderBuffer mBindlessTextureSSBO;

    // All materials
    std::vector<Material> mMaterials;
    std::unordered_map<uint32_t, MaterialMetaData> mMaterialMetaData;
    ShaderBuffer mMaterialsSSBO;

    // Async Loading
    std::vector<std::future<std::shared_ptr<LoadedModelData>>> mLoadedModelFutures;
    MainThreadTaskQueue mTaskQueue;

private:
    friend class Editor;
    friend class Renderer;
};

#endif //OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP
