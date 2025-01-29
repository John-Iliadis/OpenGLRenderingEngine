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

template <typename T>
static bool resourceLoaded(const std::unordered_map<T, std::filesystem::path>& resourceMap, const std::filesystem::path& path)
{
    return std::any_of(resourceMap.begin(), resourceMap.end(), [&path] (const auto& pair) {
        return pair.second == path;
    });
}

template <typename T>
static uint32_t sharedPtrHash(const std::shared_ptr<T> ptr) { return std::hash<std::shared_ptr<T>>()(ptr); }

uint32_t getModelID(const std::shared_ptr<Model> model) { return sharedPtrHash(model); }
uint32_t getMeshID(const std::shared_ptr<InstancedMesh> mesh) { return sharedPtrHash(mesh); }
uint32_t getTextureID(const std::shared_ptr<Texture2D> texture) { return sharedPtrHash(texture); }

ResourceManager::ResourceManager()
    : mBindlessTextureSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 1, 1024 * sizeof(uint64_t), nullptr)
    , mMaterialsSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 2, 256 * sizeof(Material), nullptr)
{
    loadDefaultTextures();
    loadDefaultMaterial();
}

ResourceManager::~ResourceManager()
{
    for (uint64_t gpuTextureHandle : mBindlessTextures)
        glMakeTextureHandleNonResidentARB(gpuTextureHandle);
}

bool ResourceManager::importModel(const std::filesystem::path &path)
{
    if (resourceLoaded(mModelPaths, path))
    {
        debugLog(std::format("Model \"{}\" is already loaded.", path.string()));
        return false;
    }

    EnqueueCallback callback = [this] (Task&& t) {mTaskQueue.push(std::move(t));};
    mLoadedModelFutures.push_back(ResourceImporter::loadModel(path, callback));

    return true;
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
void ResourceManager::addModel(std::shared_ptr<LoadedModelData> modelData)
{
    // create model
    std::shared_ptr<Model> model = std::make_shared<Model>();

    model->root = std::move(modelData->root);
    uint32_t modelID = getModelID(model);

    mModels.emplace(modelID, model);
    mModelMetaData.emplace(modelID, modelData->name);
    mModelPaths.emplace(modelID, modelData->path);

    // load textures
    std::unordered_map<uint32_t, uint32_t> insertedTextureIndexMap; // loaded texture index -> resource index
    for (uint32_t i = 0; i < modelData->textures.size(); ++i)
    {
        const auto& [texture, texturePath] = modelData->textures.at(i);

        uint32_t textureID = getTextureID(texture);
        mTextures.emplace(textureID, texture);
        mTextureMetaData.emplace(textureID, texturePath.filename().string());
        mTexturePaths.emplace(textureID, texturePath);

        uint64_t gpuTextureHandle = makeBindless(texture->id());
        mBindlessTextureMap.emplace(textureID, gpuTextureHandle);
        mBindlessTextures.push_back(gpuTextureHandle);

        insertedTextureIndexMap.emplace(i, mBindlessTextures.size() - 1);
    }

    // load materials
    std::unordered_map<uint32_t, uint32_t> insertedMaterialIndexMap; // loaded material index -> resource index
    for (uint32_t i = 0; i < modelData->materials.size(); ++i)
    {
        const LoadedModelData::Material& loadedMaterial = modelData->materials.at(i);

        int32_t baseColorTextureIndex = loadedMaterial.baseColorMapIndex;
        int32_t metallicRoughnessTextureIndex = loadedMaterial.metallicRoughnessMapIndex;
        int32_t normalTextureIndex = loadedMaterial.normalMapIndex;
        int32_t aoTextureIndex = loadedMaterial.aoMapIndex;
        int32_t emissionTextureIndex = loadedMaterial.emissionMapIndex;
        int32_t specularTextureIndex = loadedMaterial.specularGlossinessMapIndex;
        int32_t displacementTextureIndex = loadedMaterial.displacementMapIndex;

        Material material {
            .workflow = metallicRoughnessTextureIndex != -1 ? METALLIC_WORKFLOW : SPECULAR_WORKFLOW,
            .baseColorFactor = loadedMaterial.baseColorFactor,
            .emissionFactor = loadedMaterial.emissionFactor,
            .specularGlossinessFactor = loadedMaterial.specularGlossinessFactor,
            .metallicFactor = loadedMaterial.metallicFactor,
            .roughnessFactor = loadedMaterial.roughnessFactor,
            .occlusionFactor = loadedMaterial.occlusionFactor,
            .specularFactor = loadedMaterial.specularFactor
        };

        if (baseColorTextureIndex != -1)
            material.baseColorMapIndex = insertedTextureIndexMap.at(modelData->indirectTextureMap.at(baseColorTextureIndex));
        if (metallicRoughnessTextureIndex != -1)
            material.metallicRoughnessMapIndex = insertedTextureIndexMap.at(modelData->indirectTextureMap.at(metallicRoughnessTextureIndex));
        if (normalTextureIndex != -1)
            material.normalMapIndex = insertedTextureIndexMap.at(modelData->indirectTextureMap.at(normalTextureIndex));
        if (aoTextureIndex != -1)
            material.aoMapIndex = insertedTextureIndexMap.at(modelData->indirectTextureMap.at(aoTextureIndex));
        if (emissionTextureIndex != -1)
            material.emissionMapIndex = insertedTextureIndexMap.at(modelData->indirectTextureMap.at(emissionTextureIndex));
        if (specularTextureIndex != -1)
            material.specularGlossinessMapIndex = insertedTextureIndexMap.at(modelData->indirectTextureMap.at(specularTextureIndex));
        if (displacementTextureIndex != -1)
            material.displacementMapIndex = insertedTextureIndexMap.at(modelData->indirectTextureMap.at(displacementTextureIndex));

        mMaterials.push_back(material);
        mMaterialMetaData.emplace(mMaterials.size() - 1, loadedMaterial.name);

        model->indirectMaterialMap.emplace(i, loadedMaterial.name);
        model->mappedMaterials.emplace(loadedMaterial.name, mMaterials.size() - 1);

        insertedMaterialIndexMap.emplace(i, mMaterials.size() - 1);
    }

    // load meshes
    for (uint32_t i = 0; i < modelData->meshes.size(); ++i)
    {
        const LoadedModelData::Mesh& loadedMesh = modelData->meshes.at(i);

        uint32_t meshID = getMeshID(loadedMesh.mesh);
        mMeshes.emplace(meshID, loadedMesh.mesh);
        mMeshMetaData.emplace(meshID, loadedMesh.name);

        model->meshes.emplace_back(i, insertedMaterialIndexMap.at(loadedMesh.materialIndex));
        model->indirectMeshMap.emplace(i, mMeshes.size() - 1);
    }

    // update SSBOs
    mBindlessTextureSSBO.update(0, mBindlessTextures.size() * sizeof(uint64_t), mBindlessTextures.data());
    mMaterialsSSBO.update(0, mMaterials.size() * sizeof(Material), mMaterials.data());
}

void ResourceManager::deleteModel(uint32_t modelIndex)
{

}

void ResourceManager::deleteMaterial(uint32_t materialIndex)
{

}

void ResourceManager::deleteTexture(uint32_t textureIndex)
{

}

void ResourceManager::loadDefaultTextures()
{
    float textureData[DEFAULT_TEXTURE_COUNT][4] {
        {1.f, 1.f, 1.f, 1.f}, // BASE COLOR WHITE
        {0.5f, 0.5f, 0.5f, 1.f}, // BASE COLOR GREY
        {1.f, 1.f, 1.f, 1.f}, // METALLIC ROUGHNESS
        {0.5f, 0.5f, 1.f, 0.f}, // NORMAL
        {1.f, 1.f, 1.f, 1.f}, // AO
        {0.f, 0.f, 0.f, 0.f}, // EMISSION
        {1.f, 1.f, 1.f, 1.f}, // SPECULAR
        {0.f, 0.f, 0.f, 0.f} // DISPLACEMENT
    };

    const char* textureNames[] {
        "Default Base Color (White)",
        "Default Base Color (Grey)",
        "Default Metallic Roughness",
        "Default Normal",
        "Default Ambient Occlusion",
        "Default Emission",
        "Default Specular",
        "Default Displacement"
    };

    TextureSpecification textureSpecification {
        .width = 1,
        .height = 1,
        .format = TextureFormat::RGBA8,
        .dataType = TextureDataType::UINT8,
        .wrapMode = TextureWrap::Repeat,
        .filterMode = TextureFilter::Nearest,
        .generateMipMaps = false
    };

    for (uint32_t i = 0; i < DEFAULT_TEXTURE_COUNT; ++i)
    {
        auto texture = std::make_shared<Texture2D>(textureSpecification, textureData[i]);
        uint32_t textureId = getTextureID(texture);
        mTextures.emplace(textureId, texture);
        mTextureMetaData.emplace(textureId, textureNames[i]);
        uint64_t gpuTextureHandle = makeBindless(texture->id());
        mBindlessTextureMap.emplace(textureId, gpuTextureHandle);
        mBindlessTextures.push_back(gpuTextureHandle);
    }

    mBindlessTextureSSBO.update(0, mBindlessTextures.size() * sizeof(uint64_t), mBindlessTextures.data());
}

void ResourceManager::loadDefaultMaterial()
{
    Material material {
       .workflow = METALLIC_WORKFLOW,
       .baseColorMapIndex = BASE_COLOR_GREY_TEXTURE_DEFAULT_INDEX,
       .metallicRoughnessMapIndex = METALLIC_ROUGHNESS_TEXTURE_DEFAULT_INDEX,
       .normalMapIndex = NORMAL_TEXTURE_DEFAULT_INDEX,
       .aoMapIndex = AO_TEXTURE_DEFAULT_INDEX,
       .emissionMapIndex = EMISSION_TEXTURE_DEFAULT_INDEX,
       .specularGlossinessMapIndex = SPECULAR_TEXTURE_DEFAULT_INDEX,
       .displacementMapIndex = DISPLACEMENT_TEXTURE_DEFAULT_INDEX
    };

    mMaterials.push_back(material);

    mMaterialsSSBO.update(0, sizeof(Material), mMaterials.data());
}
