//
// Created by Gianni on 23/01/2025.
//

#include "resource_manager.hpp"

static uint64_t makeBindless(uint32_t textureID)
{
    uint64_t gpuTextureHandle = glGetTextureHandleARB(textureID);
    glMakeTextureHandleResidentARB(gpuTextureHandle);
    return gpuTextureHandle;
}

ResourceManager::ResourceManager()
    : mBindlessTextureSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 1, 1024 * sizeof(uint64_t), nullptr)
    , mMaterialsSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 2, 256 * sizeof(Material), nullptr)
{
    loadDefaultMaterial();
}

ResourceManager::~ResourceManager()
{
    for (uint64_t gpuTextureHandle : mBindlessTextures)
        glMakeTextureHandleNonResidentARB(gpuTextureHandle);
}

// todo: add return code for editor
void ResourceManager::importModel(const std::filesystem::path &path)
{
    if (mModels.contains(path))
    {
        debugLog(std::format("Model \"{}\" is already loaded.", path.string()));
        return;
    }

    EnqueueCallback callback = [this] (Task&& t) {mTaskQueue.push(std::move(t));};
    mLoadedModelFutures.push_back(ResourceImporter::loadModel(path, callback));
}

void ResourceManager::processMainThreadTasks()
{
    while (auto task = mTaskQueue.pop())
        (*task)();

    for (auto itr = mLoadedModelFutures.begin(); itr != mLoadedModelFutures.end();)
    {
        if (itr->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            auto loadedModel = itr->get();
            addModel(loadedModel);
            itr = mLoadedModelFutures.erase(itr);
        }
        else
            ++itr;
    }
}

// todo: check if workflow is metallic or specular
void ResourceManager::addModel(std::shared_ptr<LoadedModelData> loadedModel)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();

    mModels.emplace(loadedModel->path, model);
    mModelMetaData.emplace(model, loadedModel->modelName);

    model->root = std::move(loadedModel->root);

    // load meshes
    for (const auto& loadedMesh : loadedModel->meshes)
    {
        model->meshes.emplace_back(loadedMesh.mesh, loadedMesh.materialIndex);
        mMeshes.push_back(loadedMesh.mesh);
        mMeshMetaData.emplace(loadedMesh.mesh, loadedMesh.name);
    }

    // load textures
    std::unordered_map<std::filesystem::path, uint32_t> texturePathToIndex;
    for (const auto& [texturePath, texture] : loadedModel->textures)
    {
        // make bindless texture
        uint64_t gpuTextureHandle = makeBindless(texture->id());
        mBindlessTextures.push_back(gpuTextureHandle);
        mBindlessTextureMap.emplace(texture, gpuTextureHandle);

        // add texture
        mTextures.emplace(texturePath, texture);
        mTextureMetaData.emplace(texture, texturePath.filename().string());

        // save texture index
        texturePathToIndex.emplace(texturePath, mBindlessTextures.size() - 1);
    }

    // load materials
    for (uint32_t i = 0; i < loadedModel->materials.size(); ++i)
    {
        const LoadedModelData::Material& loadedMaterial = loadedModel->materials.at(i);

        mMaterials.emplace_back();
        Material& material = mMaterials.back();
        material.workflow = Workflow::Metallic;
        material.albedoColor = loadedMaterial.albedoColor;
        material.emissionColor = loadedMaterial.emissionColor;
        material.tiling = {1.f, 1.f};
        material.offset = {0.f, 0.f};

        auto getMaterialIndex = [&texturePathToIndex] (const std::filesystem::path& texturePath, MaterialTextureType textureType) {
            return texturePathToIndex.contains(texturePath)
                   ? texturePathToIndex.at(texturePath)
                   : static_cast<uint32_t >(textureType);
        };

        material.albedoMapIndex = getMaterialIndex(loadedMaterial.albedoTexturePath, MaterialTextureType::Albedo);
        material.specularMapIndex = getMaterialIndex(loadedMaterial.specularTexturePath, MaterialTextureType::Specular);
        material.roughnessMapIndex = getMaterialIndex(loadedMaterial.roughnessTexturePath, MaterialTextureType::Roughness);
        material.metallicMapIndex = getMaterialIndex(loadedMaterial.metallicTexturePath, MaterialTextureType::Metallic);
        material.normalMapIndex = getMaterialIndex(loadedMaterial.normalTexturePath, MaterialTextureType::Normal);
        material.displacementMapIndex = getMaterialIndex(loadedMaterial.displacementTexturePath, MaterialTextureType::Displacement);
        material.aoMapIndex = getMaterialIndex(loadedMaterial.aoTexturePath, MaterialTextureType::Ao);
        material.emissionMapIndex = getMaterialIndex(loadedMaterial.emissionTexturePath, MaterialTextureType::Emission);

        mMaterialMetaData.emplace(&material, loadedMaterial.name);

        model->indirectMatIndexMap.emplace(i, loadedMaterial.name);
        model->matNameToMatIndex.emplace(loadedMaterial.name, mMaterials.size() - 1);
    }

    // update SSBOs
    mBindlessTextureSSBO.update(0, mBindlessTextures.size() * sizeof(uint64_t), mBindlessTextures.data());
    mMaterialsSSBO.update(0, mMaterials.size() * sizeof(Material), mMaterials.data());
}

void ResourceManager::deleteModel(const std::filesystem::path &path)
{
    /* Todo list
     * remove all the nodes that use any of the deleted meshes and all their children
     * remove the model from the mModels list
     * remove the model meta data
     * remove all the meshes that are used by the mode from mMeshes
     * remove the mesh meta data used by the model
     * */
}

void ResourceManager::deleteMaterial(uint32_t materialIndex)
{
    /* Todo list
     * Remove the material from the materials vector and reformat the material array (fill deleted elements spot with the last material)
     * Remove the material meta data
     * Update the materials SSBO
     * Assign the default material to all the nodes that use that material. Additionally, update the material index for all the nodes that used the moved index.
     * */
}

void ResourceManager::deleteTexture(std::filesystem::path &path)
{
    /* Todo list
     * Remove the texture from mTextures
     * Remove the texture metadata
     * Remove the bindless texture from mBindless textures and reformat the vector
     * Delete the bindless texture
     * Update the bindless texture SSBO
     * Update all the materials that use that texture
     * */
}

void ResourceManager::loadDefaultMaterial()
{
//    float albedo[4] {0.5f, 0.5f, 0.5f, 1.f};
//    float specular[4] {0.f, 0.f, 0.f, 0.f};
//    float roughness[4] {1.f, 1.f, 1.f, 1.f};
//    float metallic[4] {0.f, 0.f, 0.f};
//    float normal[4] {0.5f, 0.5f, 1.f, 0.f};
//    float displacement[4] {0.f, 0.f, 0.f, 0.f};
//    float ao[4] {1.f, 1.f, 1.f, 1.f};
//    float emission[4] {0.f, 0.f, 0.f, 0.f};
//
//    TextureSpecification textureSpecification {
//        .width = 1,
//        .height = 1,
//        .format = TextureFormat::RGBA8,
//        .dataType = TextureDataType::UINT8,
//        .wrapMode = TextureWrap::Repeat,
//        .filterMode = TextureFilter::Nearest,
//        .generateMipMaps = false
//    };
//
//    std::shared_ptr<Texture2D> defaultAlbedoTexture = std::make_shared<Texture2D>(textureSpecification, albedo);
//    std::shared_ptr<Texture2D> defaultSpecularTexture = std::make_shared<Texture2D>(textureSpecification, specular);
//    std::shared_ptr<Texture2D> defaultRoughnessTexture = std::make_shared<Texture2D>(textureSpecification, roughness);
//    std::shared_ptr<Texture2D> defaultMetallicTexture = std::make_shared<Texture2D>(textureSpecification, metallic);
//    std::shared_ptr<Texture2D> defaultNormalTexture = std::make_shared<Texture2D>(textureSpecification, normal);
//    std::shared_ptr<Texture2D> defaultDisplacementTexture = std::make_shared<Texture2D>(textureSpecification, displacement);
//    std::shared_ptr<Texture2D> defaultAoTexture = std::make_shared<Texture2D>(textureSpecification, ao);
//    std::shared_ptr<Texture2D> defaultEmissionTexture = std::make_shared<Texture2D>(textureSpecification, emission);
//
//    mTextures.emplace(, defaultAlbedoTexture);
//    mTextures.emplace(, defaultSpecularTexture);
//    mTextures.emplace(, defaultRoughnessTexture);
//    mTextures.emplace(, defaultMetallicTexture);
//    mTextures.emplace(, defaultNormalTexture);
//    mTextures.emplace(, defaultDisplacementTexture);
//    mTextures.emplace(, defaultAoTexture);
//    mTextures.emplace(, defaultEmissionTexture);
//
//    mTextureMetaData.emplace(defaultAlbedoTexture, );
//    mTextureMetaData.emplace(defaultSpecularTexture, );
//    mTextureMetaData.emplace(defaultRoughnessTexture, );
//    mTextureMetaData.emplace(defaultMetallicTexture, );
//    mTextureMetaData.emplace(defaultNormalTexture, );
//    mTextureMetaData.emplace(defaultDisplacementTexture, );
//    mTextureMetaData.emplace(defaultAoTexture, );
//    mTextureMetaData.emplace(defaultEmissionTexture, );
//
//    uint64_t albedoTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//    uint64_t specularTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//    uint64_t roughnessTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//    uint64_t metallicTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//    uint64_t normalTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//    uint64_t displacementTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//    uint64_t aoTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//    uint64_t emissionTextureGpuHandle = makeBindless(defaultAlbedoTexture->id());
//
//    mBindlessTextureMap.emplace(defaultAlbedoTexture, albedoTextureGpuHandle);
//    mBindlessTextureMap.emplace(defaultSpecularTexture, specularTextureGpuHandle);
//    mBindlessTextureMap.emplace(defaultRoughnessTexture, roughnessTextureGpuHandle);
//    mBindlessTextureMap.emplace(defaultMetallicTexture, metallicTextureGpuHandle);
//    mBindlessTextureMap.emplace(defaultNormalTexture, normalTextureGpuHandle);
//    mBindlessTextureMap.emplace(defaultDisplacementTexture, displacementTextureGpuHandle);
//    mBindlessTextureMap.emplace(defaultAoTexture, aoTextureGpuHandle);
//    mBindlessTextureMap.emplace(defaultEmissionTexture, emissionTextureGpuHandle);
//
//    mBindlessTextures.push_back(albedoTextureGpuHandle);
//    mBindlessTextures.push_back(specularTextureGpuHandle);
//    mBindlessTextures.push_back(roughnessTextureGpuHandle);
//    mBindlessTextures.push_back(metallicTextureGpuHandle);
//    mBindlessTextures.push_back(normalTextureGpuHandle);
//    mBindlessTextures.push_back(displacementTextureGpuHandle);
//    mBindlessTextures.push_back(aoTextureGpuHandle);
//    mBindlessTextures.push_back(emissionTextureGpuHandle);
//
//    mBindlessTextureSSBO.update(0, mBindlessTextures.size() * sizeof(uint64_t ), mBindlessTextures.data());
//
//    Material material {
//        .workflow = Workflow::Metallic,
//        .albedoMapIndex = static_cast<uint32_t>(MaterialTextureType::Albedo),
//        .specularMapIndex = static_cast<uint32_t>(MaterialTextureType::Specular),
//        .roughnessMapIndex = static_cast<uint32_t>(MaterialTextureType::Roughness),
//        .metallicMapIndex = static_cast<uint32_t>(MaterialTextureType::Metallic),
//        .normalMapIndex = static_cast<uint32_t>(MaterialTextureType::Normal),
//        .displacementMapIndex = static_cast<uint32_t>(MaterialTextureType::Displacement),
//        .aoMapIndex = static_cast<uint32_t>(MaterialTextureType::Ao),
//        .emissionMapIndex = static_cast<uint32_t>(MaterialTextureType::Emission),
//        .albedoColor = glm::vec4(1.f),
//        .emissionColor = glm::vec4(0.f),
//        .tiling = glm::vec2(1.f),
//        .offset = glm::vec2(0.f)
//    };
//
//    mMaterials.push_back(material);
//
//    mMaterialsSSBO.update(0, sizeof(Material), mMaterials.data());
}
