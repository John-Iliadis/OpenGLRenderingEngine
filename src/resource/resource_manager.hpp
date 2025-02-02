//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP
#define OPENGLRENDERINGENGINE_RESOURCE_MANAGER_HPP

#include <glad/glad.h>
#include "../app/simple_notification_service.hpp"
#include "../app/uuid_registry.hpp"
#include "../opengl/shader.hpp"
#include "../renderer/material.hpp"
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

    std::shared_ptr<Model> getModel(uuid64_t id);
    std::shared_ptr<InstancedMesh> getMesh(uuid64_t id);
    std::shared_ptr<Texture> getTexture(uuid64_t id);
    uint32_t getMatIndex(uuid64_t id);

    void deleteModel(uuid64_t id);
    void deleteTexture(uuid64_t id);
    void deleteMaterial(uuid64_t id);

    std::optional<uuid64_t> getModelID(const std::shared_ptr<Model>& model);
    std::optional<uuid64_t> getMeshID(const std::shared_ptr<InstancedMesh>& mesh);
    std::optional<uuid64_t> getTextureID(const std::shared_ptr<Texture2D>& texture);

private:
    void addModel(std::shared_ptr<LoadedModelData> modelData);
    std::unordered_map<index_t, uuid64_t> addMeshes(std::shared_ptr<LoadedModelData> modelData);
    std::unordered_map<index_t, uint32_t> addTextures(std::shared_ptr<LoadedModelData> modelData);
    std::unordered_map<std::string, uuid64_t> addMaterials(std::shared_ptr<LoadedModelData> modelData,
                                                           const std::unordered_map<index_t, uint32_t>& loadedTextureIndexToResourceIndex);
    Model::Node createModelNodeHierarchy(std::shared_ptr<LoadedModelData> modelData,
                                         const LoadedModelData::Node& loadedNode,
                                         const std::unordered_map<index_t, uuid64_t>& loadedMeshIndexToMeshUUID,
                                         const std::unordered_map<std::string, uuid64_t>& loadedMatNameToMatID);

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
    std::unordered_map<uuid64_t, uint32_t> mMaterials;
    std::map<uuid64_t, std::string> mMaterialNames;
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
