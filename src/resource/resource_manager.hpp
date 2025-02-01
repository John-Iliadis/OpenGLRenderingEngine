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

class ResourceManager : public SubscriberSNS
{
public:
    ResourceManager();
    ~ResourceManager();

    bool importModel(const std::filesystem::path& path);

    void notify(const Message &message) override;

    void processMainThreadTasks();

    void deleteModel(uuid64_t id);
    void deleteTexture(uuid64_t id);
    void deleteMaterial(uuid64_t id);

    bool isModel(uuid64_t id);
    bool isMaterial(uuid64_t id);
    bool isTexture(uuid64_t id);

    std::optional<uuid64_t> getModelID(const std::shared_ptr<Model>& model);
    std::optional<uuid64_t> getMeshID(const std::shared_ptr<InstancedMesh>& mesh);
    std::optional<uuid64_t> getTextureID(const std::shared_ptr<Texture2D>& texture);

private:
    void addModel(std::shared_ptr<LoadedModelData> modelData);

    void loadDefaultTextures();
    void loadDefaultMaterial();

private:
    // All models
    std::unordered_map<uuid64_t, std::shared_ptr<Model>> mModels;
    std::unordered_map<uuid64_t, std::string> mModelNames;
    std::unordered_map<uuid64_t, std::filesystem::path> mModelPaths;

    // All meshes
    std::unordered_map<uuid64_t, std::shared_ptr<InstancedMesh>> mMeshes;
    std::unordered_map<uuid64_t, std::string> mMeshNames;

    // All textures
    std::unordered_map<uuid64_t, std::shared_ptr<Texture2D>> mTextures;
    std::unordered_map<uuid64_t, std::string> mTextureNames;
    std::unordered_map<uuid64_t, std::filesystem::path> mTexturePaths;
    std::unordered_map<uuid64_t, gpu_tex_handle64_t> mBindlessTextureMap;
    std::vector<gpu_tex_handle64_t> mBindlessTextures;
    ShaderBuffer mBindlessTextureSSBO;

    // All materials
    std::unordered_map<uuid64_t, index_t> mMaterials;
    std::unordered_map<uuid64_t, std::string> mMaterialNames;
    std::vector<Material> mMaterialArray;
    ShaderBuffer mMaterialsSSBO;

    // Async Loading
    std::vector<std::future<std::shared_ptr<LoadedModelData>>> mLoadedModelFutures;
    MainThreadTaskQueue mTaskQueue;

private:
    friend class Editor;
    friend class Renderer;
};

#endif //OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP
